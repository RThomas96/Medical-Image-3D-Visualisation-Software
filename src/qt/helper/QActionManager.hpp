#ifndef QACTION_MANAGER_HPP
#define QACTION_MANAGER_HPP

#include <map>
#include <QToolButton>
#include <QString>
#include <QStringList>
#include <QActionGroup>
#include <iostream>
#include <QMenu>

class QActionManager : QWidget {
    Q_OBJECT
public:
    std::map<std::string, QToolButton *> menus;

    std::map<std::string, QAction *> actions;
    std::map<std::string, QActionGroup *> actionExclusiveGroups;

    std::map<std::string, QStringList> actionGroups;

    QAction * getAction(const QString& name) {
        return actions[name.toStdString()];
    }

    QToolButton * getMenu(const QString& name) {
        return menus[name.toStdString()];
    }

    void activateGroup(const QString& name) {
        this->showGroup(name);
        this->enableGroup(name);
    }

    void deactivateGroup(const QString& name) {
        this->hideGroup(name);
        this->disableGroup(name);
    }

    void hideGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setVisible(false);
        }
    }

    void showGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setVisible(true);
        }
    }

    void disableGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setEnabled(false);
        }
    }

    void enableGroup(const QString& name) {
        QStringList actionsOfGroup = this->actionGroups[name.toStdString()];
        for(int i = 0; i < actionsOfGroup.size(); ++i) {
            this->actions[actionsOfGroup.at(i).toStdString()]->setEnabled(true);
        }
    }

    void createQActionGroup(const QString& name, const QStringList& actionNames, bool exclusive = false) {
        this->actionGroups[name.toStdString()] = actionNames;
    }

    void createQExclusiveActionGroup(const QString& name, const QStringList& actionNames) {
        this->actionExclusiveGroups[name.toStdString()] = new QActionGroup(this);
        this->actionExclusiveGroups[name.toStdString()]->setExclusive(true);
        for(int i = 0; i < actionNames.size(); ++i) {
            if(this->actions.count(actionNames.at(i).toStdString())) {
                this->actionExclusiveGroups[name.toStdString()]->addAction(actions[actionNames.at(i).toStdString()]);
            } else {
                std::cout << "ERROR: the action [" << actionNames.at(i).toStdString() << "] doesn't exist !" << std::endl;
                throw std::runtime_error("[ERROR] wrong action name in the function createQActionGroup.");
            }
        }
    }

    QAction * createQAction(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checkable, bool checked) {
        QIcon icon;
        QSize size(80, 80);
        if(!defaultIcon.isEmpty())
            icon.addFile(QString("../resources/icons/" + defaultIcon + QString(".svg")), size, QIcon::Normal, QIcon::Off);
        if(!pressedIcon.isEmpty())
            icon.addFile(QString("../resources/icons/" + pressedIcon + QString(".svg")), size, QIcon::Normal, QIcon::On);
    
        QAction * action = new QAction(icon, text);
        if(checkable)
            action->setCheckable(true);
        action->setStatusTip(statusTip + QString(" - ") + keySequence);
        action->setToolTip(statusTip + QString(" - ") + keySequence);
    
        action->setShortcut(QKeySequence(keySequence));
        action->setIconVisibleInMenu(true);

        actions[name.toStdString()] = action;

        if(checked) {
            action->setChecked(true);
        }
    
        return action;
    }

    void createMenuButton(const QString& name, const QString& text, const QString& statusTip, const QString& defaultIcon, const QStringList& actions) {
        QIcon icon;
        QSize size(80, 80);
        if(!defaultIcon.isEmpty())
            icon.addFile(QString("../resources/icons/" + defaultIcon + QString(".svg")), size, QIcon::Normal, QIcon::Off);

        QToolButton * button=new QToolButton(this);
        button->setStatusTip(statusTip);
        button->setToolTip(statusTip);
        button->setIcon(icon);
        this->menus[name.toStdString()] = button;
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

        button->setText(text);
        button->setPopupMode(QToolButton::InstantPopup);

        QMenu *menu=new QMenu(button);
        for(auto& actionName : actions) {
            if(actionName == QString("-"))
                menu->addSeparator();
            else
                menu->addAction(this->actions[actionName.toStdString()]);
        }
        button->setMenu(menu);
    }

    QAction * createQActionButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQAction(name, text, keySequence, statusTip, QString(), QString(), false, false);
    }

    QAction * createQActionButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, QString(), false, false);
    }

    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon, bool checked) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, checked);
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, false);
    }

    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon, const QString& pressedIcon) {
        return createQAction(name, text, keySequence, statusTip, defaultIcon, pressedIcon, true, true);
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQActionToggleButton(name, text, keySequence, statusTip, defaultIcon, QString());
    }
    
    QAction * createQActionToggleButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQActionToggleButton(name, text, keySequence, statusTip, QString(), QString());
    }

    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip, const QString& defaultIcon) {
        return createQActionToggledButton(name, text, keySequence, statusTip, defaultIcon, QString());
    }
    
    QAction * createQActionToggledButton(const QString& name, const QString& text, const QString& keySequence, const QString& statusTip) {
        return createQActionToggledButton(name, text, keySequence, statusTip, QString(), QString());
    }
};

#endif
