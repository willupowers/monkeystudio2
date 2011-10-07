#ifndef $Plugin Name.upper$_H
#define $Plugin Name.upper$_H

#include <pluginsmanager/CLIToolPlugin.h>

class $Plugin Name$ : public CLIToolPlugin
{
    Q_OBJECT
    Q_INTERFACES( BasePlugin CLIToolPlugin )

public:
    virtual pCommand defaultCommand() const;

protected:
    virtual void fillPluginInfos();

    virtual bool install();
    virtual bool uninstall();
};

#endif // $Plugin Name.upper$_H
