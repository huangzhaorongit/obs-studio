/******************************************************************************
    Copyright (C) 2018 by Hugh Bailey <obs.jim@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <QDir>
#include <QThread>
#include <QMessageBox>
#include "window-basic-main.hpp"
#include "qt-wrappers.hpp"

#include <random>

#ifdef BROWSER_AVAILABLE
#include <browser-panel.hpp>
#endif

struct QCef;
struct QCefCookieManager;

extern QCef              *cef;
extern QCefCookieManager *panel_cookies;

#ifdef BROWSER_AVAILABLE
static void InitBrowserSafeBlockEvent()
{
	QEventLoop eventLoop;

	auto wait = [&] ()
	{
		cef->wait_for_browser_init();
		QMetaObject::invokeMethod(&eventLoop, "quit",
				Qt::QueuedConnection);
	};

	QScopedPointer<QThread> thread(CreateQThread(wait));
	thread->start();
	eventLoop.exec();
	thread->wait();
}

static void InitBrowserSafeBlockMsgBox()
{
	QMessageBox dlg;
	dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowCloseButtonHint);
	dlg.setWindowTitle(QTStr("BrowserPanelInit.Title"));
	dlg.setText(QTStr("BrowserPanelInit.Text"));
	dlg.setStandardButtons(0);

	auto wait = [&] ()
	{
		cef->wait_for_browser_init();
		QMetaObject::invokeMethod(&dlg, "accept", Qt::QueuedConnection);
	};

	QScopedPointer<QThread> thread(CreateQThread(wait));
	thread->start();
	dlg.exec();
	thread->wait();
}

void CheckExistingCookieId();

static void InitPanelCookieManager()
{
	if (!cef)
		return;
	if (panel_cookies)
		return;

	CheckExistingCookieId();

	OBSBasic *main = OBSBasic::Get();
	const char *cookie_id = config_get_string(main->Config(),
			"Panels", "CookieId");

	std::string sub_path;
	sub_path += "obs_profile_cookies/";
	sub_path += cookie_id;

	panel_cookies = cef->create_cookie_manager(sub_path);
}
#endif

void OBSBasic::InitBrowserPanelSafeBlock(bool showDialog)
{
#ifdef BROWSER_AVAILABLE
	if (!cef)
		return;
	if (cef->init_browser()) {
		InitPanelCookieManager();
		return;
	}

	if (showDialog)
		InitBrowserSafeBlockMsgBox();
	else
		InitBrowserSafeBlockEvent();
	InitPanelCookieManager();
#else
	UNUSED_PARAMETER(showDialog);
#endif
}

void DestroyPanelCookieManager()
{
#ifdef BROWSER_AVAILABLE
	if (panel_cookies) {
		panel_cookies->FlushStore();
		delete panel_cookies;
		panel_cookies = nullptr;
	}
#endif
}

static std::string GenId()
{
	std::random_device rd;
	std::mt19937_64 e2(rd());
	std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);

	uint64_t id = dist(e2);

	char id_str[20];
	snprintf(id_str, sizeof(id_str), "%16llX", (unsigned long long)id);
	return std::string(id_str);
}

void DuplicateCurrentCookieProfile(ConfigFile &config)
{
#ifdef BROWSER_AVAILABLE
	if (panel_cookies) {
		OBSBasic *main = OBSBasic::Get();
		const char *cookie_id = config_get_string(main->Config(),
				"Panels", "CookieId");

		std::string src_path;
		src_path += "obs_profile_cookies/";
		src_path += cookie_id;

		std::string new_id = GenId();

		std::string dst_path;
		dst_path += "obs_profile_cookies/";
		dst_path += new_id;
		dst_path = cef->get_cookie_path(dst_path);

		BPtr<char> src_path_full = cef->get_cookie_path(src_path);
		BPtr<char> dst_path_full = cef->get_cookie_path(dst_path);

		QDir srcDir(src_path_full.Get());
		QDir dstDir(dst_path_full.Get());

		if (!srcDir.exists())
			return;
		if (!dstDir.exists())
			dstDir.mkdir(dst_path_full.Get());

		QStringList files = srcDir.entryList(QDir::Files);
		for (const QString &file : files) {
			QString src = QString(src_path_full);
			QString dst = QString(dst_path_full);
			src += QDir::separator() + file;
			dst += QDir::separator() + file;
			QFile::copy(src, dst);
		}

		config_set_string(config, "Panels", "CookieId", new_id.c_str());
	}
#else
	UNUSED_PARAMETER(newProfile);
#endif
}

void CheckExistingCookieId()
{
	OBSBasic *main = OBSBasic::Get();
	if (config_has_user_value(main->Config(), "Panels", "CookieId"))
		return;

	config_set_string(main->Config(), "Panels", "CookieId", GenId().c_str());
}
