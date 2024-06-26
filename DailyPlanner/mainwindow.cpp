#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDate nextDate = currentDate.addDays(1);
    QDate previousDay = currentDate.addDays(-1);
    ui->dayLabel1->setText(previousDay.toString("yyyy-MM-dd"));
    ui->dayLabel2->setText(currentDate.toString("yyyy-MM-dd"));
    ui->dayLabel3->setText(nextDate.toString("yyyy-MM-dd"));

    getValueFromDb();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::rmDbItem(Task *task)
{
    if (db.connect(path))
    {
        QString condition = "startTime=" + QString::number(task->getIndexTime()) + " AND days=" + QString::number(task->getDaySince());
        db.deleteData("tasks", condition);
        qInfo() << "xddddddddddddddd" << task->getIndexTime() << task->getDaySince();
        db.disconnect();
    }

}


void MainWindow::on_nextDatButton_clicked()
{
    currentDate = currentDate.addDays(1);
    QDate nextDate = currentDate.addDays(1);
    QDate previousDay = currentDate.addDays(-1);
    ui->dayLabel1->setText(previousDay.toString("yyyy-MM-dd"));
    ui->dayLabel2->setText(currentDate.toString("yyyy-MM-dd"));
    ui->dayLabel3->setText(nextDate.toString("yyyy-MM-dd"));
    getValueFromDb();
}


void MainWindow::on_previousDayButton_clicked()
{
    currentDate = currentDate.addDays(-1);
    QDate nextDate = currentDate.addDays(1);
    QDate previousDay = currentDate.addDays(-1);
    ui->dayLabel1->setText(previousDay.toString("yyyy-MM-dd"));
    ui->dayLabel2->setText(currentDate.toString("yyyy-MM-dd"));
    ui->dayLabel3->setText(nextDate.toString("yyyy-MM-dd"));
    getValueFromDb();
}



void MainWindow::on_addDayWindow1_clicked()
{
    QTime startTime = ui->timeStartWindow1->time();
    QTime endTime = ui->timeEndWindow1->time();
    int startTimeInt = startTime.msecsSinceStartOfDay() / (1000 * 60);
    int endTimeInt = endTime.msecsSinceStartOfDay() / (1000 * 60);
    if (startTime >= endTime)
    {
        QMessageBox::information(nullptr, "Problem", "Czas startu nie może być mniejszy niż czas końca czynności");
        return;
    }
    QString text = ui->textWindow1->text();
    if (text.isEmpty())
    {
        QMessageBox::information(nullptr, "Problem", "Treść zadania nie może być pusta");
        return;
    }
    QDate previousDay = currentDate.addDays(-1);
    int julianDay = previousDay.toJulianDay();
    if (!addTaskToDataBase(startTimeInt, endTimeInt, text, julianDay))
    {
        QMessageBox::information(nullptr, "Problem", "Czas zadania pokrywa się z istniejącym zadaniem");
        return;
    }
    QString timeText = startTime.toString("HH:mm") + "->" + endTime.toString("HH:mm");
    Task *task = new Task(nullptr, ui->textWindow1->text(), timeText, startTimeInt, julianDay);
    connect(this, &MainWindow::removeAll, task, &Task::removeAllSlot);
    connect(task, &Task::removeFromDb, this, &MainWindow::rmDbItem);
    ui->day1layout->insertWidget(getIndexToSort(ui->day1layout, startTimeInt), task);
//    ui->day1layout->addWidget(task);
    ui->textWindow1->clear();
}


void MainWindow::on_addDayWindow2_clicked()
{
    QTime startTime = ui->timeStartWindow2->time();
    QTime endTime = ui->timeEndWindow2->time();
    int startTimeInt = startTime.msecsSinceStartOfDay() / (1000 * 60);
    int endTimeInt = endTime.msecsSinceStartOfDay() / (1000 * 60);
    if (startTime >= endTime)
    {
        QMessageBox::information(nullptr, "Problem", "Czas startu nie może być mniejszy niż czas końca czynności");
        return;
    }
    QString text = ui->textWindow2->text();
    if (text.isEmpty())
    {
        QMessageBox::information(nullptr, "Problem", "Treść zadania nie może być pusta");
        return;
    }
    int julianDay = currentDate.toJulianDay();
    if (!addTaskToDataBase(startTimeInt, endTimeInt, text, julianDay))
    {
        QMessageBox::information(nullptr, "Problem", "Czas zadania pokrywa się z istniejącym zadaniem");
        return;
    }
    QString timeText = startTime.toString("HH:mm") + "->" + endTime.toString("HH:mm");
    Task *task = new Task(nullptr, ui->textWindow2->text(), timeText, startTimeInt, julianDay);
    connect(this, &MainWindow::removeAll, task, &Task::removeAllSlot);
    connect(task, &Task::removeFromDb, this, &MainWindow::rmDbItem);
    ui->day2layout->insertWidget(getIndexToSort(ui->day2layout, startTimeInt), task);
    ui->textWindow2->clear();
}


void MainWindow::on_addDayWindow3_clicked()
{
    QTime startTime = ui->timeStartWindow3->time();
    QTime endTime = ui->timeEndWindow3->time();
    int startTimeInt = startTime.msecsSinceStartOfDay() / (1000 * 60);
    int endTimeInt = endTime.msecsSinceStartOfDay() / (1000 * 60);
    if (startTime >= endTime)
    {
        QMessageBox::information(nullptr, "Problem", "Czas startu nie może być mniejszy niż czas końca czynności");
        return;
    }
    QString text = ui->textWindow3->text();
    if (text.isEmpty())
    {
        QMessageBox::information(nullptr, "Problem", "Treść zadania nie może być pusta");
        return;
    }
    QDate nextDay = currentDate.addDays(-1);
    int julianDay = nextDay.toJulianDay();
    if (!addTaskToDataBase(startTimeInt, endTimeInt, text, julianDay))
    {
        QMessageBox::information(nullptr, "Problem", "Czas zadania pokrywa się z istniejącym zadaniem");
        return;
    }
    QString timeText = startTime.toString("HH:mm") + "->" + endTime.toString("HH:mm");
    Task *task = new Task(nullptr, ui->textWindow3->text(), timeText, startTimeInt, julianDay);
    connect(this, &MainWindow::removeAll, task, &Task::removeAllSlot);
    connect(task, &Task::removeFromDb, this, &MainWindow::rmDbItem);
    ui->day3layout->insertWidget(getIndexToSort(ui->day3layout, startTimeInt), task);
    ui->textWindow3->clear();
}

bool MainWindow::addTaskToDataBase(int timeStart, int timeEnd, QString text, int day)
{
    if (db.connect(path))
    {
        QList<QList<QString>> data = db.fetchData("tasks");
        for (const auto &row : data)
        {
//            qInfo() << day << row.at()
            if (day != row.at(3).toInt())
                continue;
            qInfo() << "xd" << timeStart << row.at(1).toInt() << row.at(0).toInt();
            if (timeStart >= row.at(0).toInt() and timeStart < row.at(1).toInt())
            {
                db.disconnect();
                return false;
            }
            if (timeEnd > row.at(0).toInt() and timeEnd <= row.at(1).toInt())
            {
                db.disconnect();
                return false;
            }
        }
        QStringList values = {
            QString::number(timeStart),
            QString::number(timeEnd),
            QString("'%1'").arg(text),
            QString::number(day)
        };

        db.insertData("tasks", values);
        db.disconnect();
        return true;
    }

    return false;
}

int MainWindow::getIndexToSort(QHBoxLayout *layout, int timeActualItem)
{
    int insertIndex = 0;
    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget) {
            Task *taskInLayout = qobject_cast<Task*>(widget);
            if (taskInLayout && taskInLayout->getIndexTime() < timeActualItem) {
                insertIndex = i + 1;
            }
        }
    }
    return insertIndex;
}

void MainWindow::getValueFromDb()
{
    emit removeAll();

    if (db.connect(path))
    {
        QList<QList<QString>> data = db.fetchData("tasks");
        int julianDay = currentDate.toJulianDay();
        for (const auto &row : data) {
            if (julianDay - 1 == row.at(3).toInt()) // prev
            {
                QTime startTime = QTime(row.at(0).toInt() / 60, row.at(0).toInt() % 60);
                QTime endTime = QTime(row.at(1).toInt() / 60, row.at(1).toInt() % 60);
                QString timeText = startTime.toString("HH:mm") + "->" + endTime.toString("HH:mm");
                Task *task = new Task(nullptr, row.at(2), timeText, row.at(0).toInt(), row.at(3).toInt());
                connect(this, &MainWindow::removeAll, task, &Task::removeAllSlot);
                connect(task, &Task::removeFromDb, this, &MainWindow::rmDbItem);
                ui->day1layout->insertWidget(getIndexToSort(ui->day1layout, row.at(0).toInt()), task);
            }
            else if (julianDay == row.at(3).toInt()) // act
            {
                QTime startTime = QTime(row.at(0).toInt() / 60, row.at(0).toInt() % 60);
                QTime endTime = QTime(row.at(1).toInt() / 60, row.at(1).toInt() % 60);
                QString timeText = startTime.toString("HH:mm") + "->" + endTime.toString("HH:mm");
                Task *task = new Task(nullptr, row.at(2), timeText, row.at(0).toInt(), row.at(3).toInt());
                connect(this, &MainWindow::removeAll, task, &Task::removeAllSlot);
                connect(task, &Task::removeFromDb, this, &MainWindow::rmDbItem);
                ui->day2layout->insertWidget(getIndexToSort(ui->day2layout, row.at(0).toInt()), task);
            }
            else if (julianDay + 1 == row.at(3).toInt()) // next
            {
                QTime startTime = QTime(row.at(0).toInt() / 60, row.at(0).toInt() % 60);
                QTime endTime = QTime(row.at(1).toInt() / 60, row.at(1).toInt() % 60);
                QString timeText = startTime.toString("HH:mm") + "->" + endTime.toString("HH:mm");
                Task *task = new Task(nullptr, row.at(2), timeText, row.at(0).toInt(), row.at(3).toInt());
                connect(this, &MainWindow::removeAll, task, &Task::removeAllSlot);
                connect(task, &Task::removeFromDb, this, &MainWindow::rmDbItem);
                ui->day3layout->insertWidget(getIndexToSort(ui->day3layout, row.at(0).toInt()), task);
            }
        }
        db.disconnect();
    }
}

void MainWindow::exportDataToFile(QDate date)
{
    QString dateStr = date.toString("yyyy-MM-dd");
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save File", dateStr, "Text Files (*.txt)");
    std::ofstream outputFile(fileName.toStdString());
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }
    outputFile << dateStr.toStdString() << std::endl;
    outputFile <<"Index\tPoczątek czasu\tKoniec czasu\tZadanie"  << std::endl;
    if (db.connect(path))
    {
        int julianDay = date.toJulianDay();
        QList<QList<QString>> data = db.fetchData("tasks");
        int index = 1;
        for (const auto &row : data)
        {
            if (julianDay  == row.at(3).toInt())
            {
                QTime startTime = QTime(row.at(0).toInt() / 60, row.at(0).toInt() % 60);
                QString startTimeText = startTime.toString("HH:mm");
                QTime endTime = QTime(row.at(1).toInt() / 60, row.at(1).toInt() % 60);
                QString endTimeText = startTime.toString("HH:mm");
                outputFile << index << "\t" << startTimeText.toStdString() << "\t" << endTimeText.toStdString() << "\t" << row.at(2).toStdString()<< std::endl;
                index++;
            }
        }
        db.disconnect();
    }
    outputFile.close();
    std::cout << "Data exported to " << "output.txt" << " successfully." << std::endl;
}


void MainWindow::on_generate1_clicked()
{
    exportDataToFile(currentDate.addDays(-1));
}


void MainWindow::on_generate2_clicked()
{
    exportDataToFile(currentDate);
}


void MainWindow::on_generate3_clicked()
{
    exportDataToFile(currentDate.addDays(1));
}

