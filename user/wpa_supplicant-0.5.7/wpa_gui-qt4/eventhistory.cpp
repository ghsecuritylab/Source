/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * wpa_gui - EventHistory class
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include <QHeaderView>

#include "eventhistory.h"


int EventListModel::rowCount(const QModelIndex &) const
{
	return msgList.count();
}


int EventListModel::columnCount(const QModelIndex &) const
{
	return 2;
}


QVariant EventListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

        if (role == Qt::DisplayRole)
		if (index.column() == 0) {
			if (index.row() >= timeList.size())
				return QVariant();
			return timeList.at(index.row());
		} else {
			if (index.row() >= msgList.size())
				return QVariant();
			return msgList.at(index.row());
		}
        else
		return QVariant();
}


QVariant EventListModel::headerData(int section, Qt::Orientation orientation,
				    int role) const
{
	if (role != Qt::DisplayRole)
		return QVariant();

	if (orientation == Qt::Horizontal) {
		switch (section) {
		case 0:
			return QString("Timestamp");
		case 1:
			return QString("Message");
		default:
			return QVariant();
		}
	} else
		return QString("%1").arg(section);
}


void EventListModel::addEvent(QString time, QString msg)
{
	beginInsertRows(QModelIndex(), msgList.size(), msgList.size() + 1);
	timeList << time;
	msgList << msg;
	endInsertRows();
}


EventHistory::EventHistory(QWidget *parent, const char *, bool, Qt::WFlags)
	: QDialog(parent)
{
	setupUi(this);

	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	elm = new EventListModel(parent);
	eventListView->setModel(elm);
}


EventHistory::~EventHistory()
{
	destroy();
	delete elm;
}


void EventHistory::languageChange()
{
	retranslateUi(this);
}


void EventHistory::addEvents(WpaMsgList msgs)
{
	WpaMsgList::iterator it;
	for (it = msgs.begin(); it != msgs.end(); it++)
		addEvent(*it);
}


void EventHistory::addEvent(WpaMsg msg)
{
	elm->addEvent(msg.getTimestamp().toString("yyyy-MM-dd hh:mm:ss.zzz"),
		      msg.getMsg());
#if QT_VERSION >= 0x040100
	eventListView->resizeColumnsToContents();
	eventListView->resizeRowsToContents();
#endif
}
