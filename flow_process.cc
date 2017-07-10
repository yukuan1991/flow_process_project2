﻿#include "flow_process.h"
#include "ui_flow_process.h"
#include "interface_control/about_us_dlg.h"
#include "interface_control/canvas_view.h"
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPrintDialog>
#include <QFileDialog>
#include <base/io/file/file.hpp>
#include <base/utils/charset.hpp>
#include "interface_control/time_unit_dlg.h"
#include "interface_control/distance_unit_dlg.h"

#include <QDebug>

flow_process::flow_process(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::flow_process),
    time_(new time_unit_dlg(this)),
    distance_(new distance_unit_dlg(this))
{
    ui->setupUi(this);
    ui->mdiarea->setViewMode(QMdiArea::TabbedView);
    init_conn();
    set_button_enabled();
    setMinimumSize(1000, 800);
    setWindowIcon(QIcon("png/flow_process.png"));
}

flow_process::~flow_process()
{
    delete ui;
}

void flow_process::file_operations(const QString &s)
{
    if(s == "新建")
    {
        file_new();
    }
    else if(s == "打开")
    {
        file_open();
    }
    else if(s == "保存")
    {
        file_save();
    }
    else if(s == "另存为")
    {
        file_save_as();
    }
    else if(s == "打印")
    {
        file_print();
    }
    else if(s == "退出")
    {
        close();
    }
}

void flow_process::file_new()
{
    auto canvas = create_canvas_view();

    canvas->show();
}

void flow_process::file_open()
{
    auto file_name = QFileDialog::getOpenFileName(this, "打开文件", ".", "Images (*.fps)");
    if (file_name.isEmpty())
    {
        return;
    }

    auto file_content = file::read_all(::utf_to_sys(file_name.toStdString()).data());
    if (!file_content)
    {
        QMessageBox::information(this, "打开", "打开文件失败，请检查文件是否存在");
        return;
    }

    auto canvas = create_canvas_view();
    if (!canvas->load (*file_content))
    {
        QMessageBox::information(this, "打开", "打开文件失败，文件已经损坏");
        return;
    }

    canvas->set_attached_file(std::move (file_name));
    canvas->ensureVisible(0, 0, 300,200 ); //打开文件时确保显示画布的区域在左上角
}

void flow_process::file_save()
{
    auto sub_window = ui->mdiarea->activeSubWindow();
    assert (sub_window);
    save_subwindow (sub_window);
}

void flow_process::file_save_as()
{
    auto sub_window = ui->mdiarea->activeSubWindow();
    assert(sub_window);

    auto save_as_view = dynamic_cast<canvas_view*>(sub_window->widget());
    assert(sub_window);

    auto path = QFileDialog::getSaveFileName(this, "另存为", ".", "Flow Process Sheet (*.fps)");
    if(path.isEmpty())
    {
        return;
    }

    file::write_buffer(::utf_to_sys(path.toStdString()).data(), save_as_view->dump());
    save_as_view->set_attached_file(std::move(path));
    emit save_as_view->saved();
}

void flow_process::save_subwindow(QMdiSubWindow *sub_window)
{
    auto save_view = dynamic_cast<canvas_view*>(sub_window->widget());
    assert(save_view);

    QString path;
    if(save_view->attached_file().isEmpty())
    {
        path = QFileDialog::getSaveFileName(this, "文件保存", ".", "Flow Process Sheet (*.fps)");
        if (path.isEmpty())
        {
            return;
        }
    }
    else
    {
        path = save_view->attached_file();
    }

    file::write_buffer(::utf_to_sys(path.toStdString()).data(), save_view->dump());
    save_view->set_attached_file(std::move(path));

    emit save_view->saved();
}

void flow_process::file_print()
{
    auto view = active_canvas_view();
    if(view == nullptr)
    {
        QMessageBox::information(this, "打印", "没有选中的窗口");
        return;
    }

    QPrinter printer;
    printer.setOrientation(QPrinter::Portrait);
    printer.setPageSize(QPrinter::A4);
    QPrintDialog dlg(&printer);
    if(QPrintDialog::Accepted == dlg.exec())
    {
        view->print_render(&printer);
    }
}

void flow_process::set_time_unit()
{
    time_->exec();
}

void flow_process::set_distance_unit()
{
    distance_->exec();
}

void flow_process::help_advice()
{
    const QString text = R"(<html><head/><body><p>如果您有任何需求与改进建议，</p><p>请随时联系IEToolkit君qq3350436646</p>
                         <p><span style=" font-weight:600; color:red">加好友请扫右边二维码---&gt;</span></p></body></html>)";
    const QString qr_code = R"( <a href = "www.shionto.com"> <img src="./png/about-us.png" width="300" height = "400"/></a>)";
    about_us_dlg::show_info(text, qr_code);
}

canvas_view *flow_process::create_canvas_view()
{
    auto canvas = canvas_view::make (ui->mdiarea);

    auto ptr_canvas = canvas.get();

    canvas->setAttribute (Qt::WA_DeleteOnClose);
    canvas->setWindowState(Qt::WindowMaximized);

    ui->mdiarea->addSubWindow(canvas.release());
    connect(time_, &time_unit_dlg::currenttextchanged, ptr_canvas, &canvas_view::time_unit_changed);
    connect(distance_, &distance_unit_dlg::currenttextchanged, ptr_canvas, &canvas_view::distance_unit_changed);

    connect(ptr_canvas, &canvas_view::load_distance_unit, distance_, &distance_unit_dlg::set_unit);
    connect(ptr_canvas, &canvas_view::load_time_unit, [] (const QString& s) { qDebug() << "flow_process" << s;});
    connect(ptr_canvas, &canvas_view::load_time_unit, time_, &time_unit_dlg::set_unit);


//    connect(ptr_canvas, &canvas_view::view_closed, this, &flow_process::on_view_closed, Qt::QueuedConnection);
    return ptr_canvas;
}

canvas_view *flow_process::active_canvas_view()
{
    if(QMdiSubWindow* active_subwindow = ui->mdiarea->activeSubWindow())
    {
        canvas_view* canvas_ptr = dynamic_cast<canvas_view*>(active_subwindow->widget());
        return canvas_ptr;
    }
    else
    {
        return nullptr;
    }
}

void flow_process::init_conn()
{
    connect(ui->flowprocess_ribbon, &flow_process_ribbon::file_menu_triggered, [this] (const QString & s) { file_operations(s); });
    connect(ui->flowprocess_ribbon, &flow_process_ribbon::time_unit, this, &flow_process::set_time_unit);
    connect(ui->flowprocess_ribbon, &flow_process_ribbon::distance_unit, this, &flow_process::set_distance_unit);
    connect(ui->flowprocess_ribbon, &flow_process_ribbon::help, this, &flow_process::help_advice);
    connect(ui->mdiarea, &QMdiArea::subWindowActivated, this, &flow_process::set_button_enabled);


}

void flow_process::set_button_enabled()
{
    const bool has_canvas_view = (active_canvas_view() != nullptr);
    ui->flowprocess_ribbon->set_enabled(has_canvas_view);
}
