#include "IgnoresPage.hpp"

#include "Application.hpp"
#include "controllers/accounts/AccountController.hpp"
#include "controllers/ignores/IgnoreController.hpp"
#include "controllers/ignores/IgnoreEmoteModel.hpp"
#include "controllers/ignores/IgnoreModel.hpp"
#include "providers/twitch/TwitchAccount.hpp"
#include "singletons/Settings.hpp"
#include "util/LayoutCreator.hpp"
#include "widgets/helper/EditableModelView.hpp"

#include <QCheckBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

// clang-format off
#define INFO "/ignore <user> in chat ignores a user.\n/unignore <user> in chat unignores a user.\nYou can also click on a user to open the usercard."
// clang-format on

namespace chatterino {

static void addEmotesTab(LayoutCreator<QVBoxLayout> box);
static void addPhrasesTab(LayoutCreator<QVBoxLayout> box);
static void addUsersTab(IgnoresPage &page, LayoutCreator<QVBoxLayout> box,
                        QStringListModel &model);

IgnoresPage::IgnoresPage()
{
    LayoutCreator<IgnoresPage> layoutCreator(this);
    auto layout = layoutCreator.setLayoutType<QVBoxLayout>();
    auto tabs = layout.emplace<QTabWidget>();

    addPhrasesTab(tabs.appendTab(new QVBoxLayout, "Messages"));
    addUsersTab(*this, tabs.appendTab(new QVBoxLayout, "Users"),
                this->userListModel_);
    addEmotesTab(tabs.appendTab(new QVBoxLayout, "Emotes"));
}

void addEmotesTab(LayoutCreator<QVBoxLayout> layout)
{
    layout.emplace<QLabel>("Ignore specified emotes from autocompletion.");
    EditableModelView *view =
        layout
            .emplace<EditableModelView>(
                (new IgnoreEmoteModel(nullptr))
                    ->initialized(&getSettings()->ignoredEmotes))
            .getElement();
    view->setTitles({"Emote Name"});
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Fixed);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    view->addRegexHelpLink();

    QTimer::singleShot(1, [view] {
        view->getTableView()->resizeColumnsToContents();
        view->getTableView()->setColumnWidth(0, 200);
    });

    view->addButtonPressed.connect(
        [] { getSettings()->ignoredEmotes.append(QString("emote name")); });
}

void addPhrasesTab(LayoutCreator<QVBoxLayout> layout)
{
    layout.emplace<QLabel>("Ignore messages based certain patterns.");
    EditableModelView *view =
        layout
            .emplace<EditableModelView>(
                (new IgnoreModel(nullptr))
                    ->initialized(&getSettings()->ignoredMessages))
            .getElement();
    view->setTitles(
        {"Pattern", "User", "Regex", "Case Sensitive", "Block", "Replacement"});
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Fixed);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    view->addRegexHelpLink();

    QTimer::singleShot(1, [view] {
        view->getTableView()->resizeColumnsToContents();
        view->getTableView()->setColumnWidth(0, 200);
    });

    view->addButtonPressed.connect([] {
        getSettings()->ignoredMessages.append(
            IgnorePhrase{"my phrase", "user", false, false,
                         getSettings()->ignoredPhraseReplace.getValue(), true});
    });
}

void addUsersTab(IgnoresPage &page, LayoutCreator<QVBoxLayout> users,
                 QStringListModel &userModel)
{
    auto label = users.emplace<QLabel>(INFO);
    label->setWordWrap(true);
    users.append(page.createCheckBox("Enable twitch ignored users",
                                     getSettings()->enableTwitchIgnoredUsers));

    auto anyways = users.emplace<QHBoxLayout>().withoutMargin();
    {
        anyways.emplace<QLabel>("Show messages from ignored users anyways:");

        auto combo = anyways.emplace<QComboBox>().getElement();
        combo->addItems(
            {"Never", "If you are Moderator", "If you are Broadcaster"});

        auto &setting = getSettings()->showIgnoredUsersMessages;

        setting.connect([combo](const int value) {
            combo->setCurrentIndex(value);
        });

        QObject::connect(combo,
                         QOverload<int>::of(&QComboBox::currentIndexChanged),
                         [&setting](int index) {
                             if (index != -1)
                                 setting = index;
                         });

        anyways->addStretch(1);
    }

    /*auto addremove = users.emplace<QHBoxLayout>().withoutMargin();
    {
        auto add = addremove.emplace<QPushButton>("Ignore user");
        auto remove = addremove.emplace<QPushButton>("Unignore User");
        addremove->addStretch(1);
    }*/

    users.emplace<QLabel>("List of ignored users:");
    users.emplace<QListView>()->setModel(&userModel);
}

void IgnoresPage::onShow()
{
    auto app = getApp();

    auto user = app->accounts->twitch.getCurrent();

    if (user->isAnon())
    {
        return;
    }

    QStringList users;
    for (const auto &ignoredUser : user->getIgnores())
    {
        users << ignoredUser.name;
    }
    users.sort(Qt::CaseInsensitive);
    this->userListModel_.setStringList(users);
}

}  // namespace chatterino
