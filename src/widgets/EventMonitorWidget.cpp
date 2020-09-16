#include "EventMonitorWidget.h"
#include "EventMonitor.h"
#include "EventMessage.h"
#include "main.h"
#include "log.h"
#include <QSvgGenerator>

EventMonitorWidget::EventMonitorWidget(EventMonitor * monitor)
{
    QVBoxLayout * layout = new QVBoxLayout();
    setLayout(layout);

    layout->addWidget(&messagesWidget_);

    // make connections
    connect(monitor, SIGNAL(clearMessages()), this, SLOT(clearMessages()));

    connect(monitor, SIGNAL(newEventMessage(boost::shared_ptr<EventMessage>)), this, SLOT(insertEventMessage(boost::shared_ptr<EventMessage>)));
}

EventMonitorWidget::~EventMonitorWidget()
{
    // the tempory file doesn't have the extension as written by the SVG exporter
    // so, we need to delete the file with the extension, too
    if(svgTmpFile_.fileName().isEmpty() != true)
    {
        QFile svgTmpFileOther(svgTmpFile_.fileName() + ".svg");

        if(svgTmpFileOther.exists() == true && svgTmpFileOther.remove() == true)
        {
            put_flog(LOG_DEBUG, "removed temporary file %s", svgTmpFileOther.fileName().toStdString().c_str());
        }
        else
        {
            put_flog(LOG_ERROR, "could not remove temporary file %s", svgTmpFileOther.fileName().toStdString().c_str());
        }
    }
}


void EventMonitorWidget::clearMessages()
{
    messagesWidget_.clear();
}

void EventMonitorWidget::insertEventMessage(boost::shared_ptr<EventMessage> message)
{
    // move cursor to beginning
    QTextCursor cursor = messagesWidget_.textCursor();
    cursor.setPosition(0);
    messagesWidget_.setTextCursor(cursor);

    messagesWidget_.insertHtml(message->messageText.c_str() + QString("<br />"));
}
