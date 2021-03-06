﻿#pragma once
#include <QGraphicsScene>
#include <memory>
#include <vector>
#include "time_unit_dlg.h"
#include "distance_unit_dlg.h"
#include "openvaf.h"
#include <QDebug>

using std::unique_ptr;
using std::make_unique;

class symbol_item;
class QLineEdit;
class table_widget;


class canvas_scene : public QGraphicsScene
{
    Q_OBJECT
signals:
    void item_selection_changed (int row, int col, bool selected);
    void scene_changed();
    void load_hide_state(bool);
public:
    void time_unit_exec();
    void distance_unit_exec();
    void hide(bool);
    void hide_current_state() { emit load_hide_state(hide_state_); }
    void copy();
    void cut();
    void paste();
    void del();
public:
    template<typename ... ARGS> static unique_ptr<canvas_scene> make (ARGS && ...args);

    QRectF print_rect () { return print_rect_; }
    //auto times(int col) { return times_[col]; }

    std::string dump ();
    bool load (const std::string& data);
    bool import(const std::vector<taskInfo>& data);
    ~canvas_scene() { qDebug() << "~canvas_scene()"; }
protected:
    void drawBackground(QPainter* painter, const QRectF &rect) override;
protected:
    template<typename ... ARGS>
    canvas_scene(ARGS && ... args) : QGraphicsScene(std::forward<ARGS> (args)...) {}

    bool init();
    void set_table();
    void add_symbol_item();

    void draw_table(QPainter* painter);
    void draw_table_text(QPainter* painter);
    void draw_symbol(QPainter* painter);
    void draw_connected_lines (QPainter * painter);
    void draw_counts (QPainter* painter);
    void draw_times (QPainter* painter);
    void draw_distance (QPainter* painter);
private:
    void time_unit_confirm();
    void time_unit_cancel();
    void distance_unit_confirm();
    void distance_unit_cancel();
private:
    void unselect_other (int row, int col, bool selected);
private:
    std::vector<std::vector<QLineEdit*>> second_table_edit_;
    std::vector<std::vector<symbol_item*>> icons_;

    table_widget* tablewidget_;
    unique_ptr<time_unit_dlg> dlg_time_unit_;
    unique_ptr<distance_unit_dlg> dlg_distance_unit_;
    QString str_time_unit_;
    QString str_distance_unit_;
    bool hide_state_ = false;
private:
    QRectF print_rect_;
private:

    static constexpr auto width_ = 750;
    static constexpr auto line_height_ = 30;
    static constexpr auto first_table_height_ = line_height_ * 4;
    static constexpr auto second_table_height_ = line_height_ * 27;
    static constexpr auto serial_number_width_ = 45;
    static constexpr auto job_content_width_ = 150;
    static constexpr auto distance_width_ = 90;
    static constexpr auto time_width_ = 90;

    static constexpr auto first_table_row_size_= 4;
    static constexpr auto first_table_rows_size_ = 5;
    static constexpr auto first_table_front_col_ = 1;
    static constexpr auto first_table_behind_col_ = 5;
    static constexpr auto second_table_row_size_ = 25;
    static constexpr auto second_table_front_col = 3;
    static constexpr auto second_table_behind_col = 5;


    /*画布左上角到第一张表左上角的坐标偏移量*/
    static constexpr auto offset_topleft_width_ =  0;
    static constexpr auto offset_topleft_height_ = 0;

    static const QPointF top_left;
private:
    std::array<int, 5> times_ {{0}};
};


template<typename ... ARGS>
unique_ptr<canvas_scene> canvas_scene::make (ARGS && ...args)
{
    unique_ptr<canvas_scene> ret (new canvas_scene (std::forward<ARGS> (args)...));
    if(ret == nullptr or !ret->init())
    {
        return nullptr;
    }

    return ret;
}
