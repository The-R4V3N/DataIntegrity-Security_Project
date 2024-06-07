# This directory is for project specific (private) libraries

The source code of each library is be placed in a an own separate directory
("lib/communication[communication.py]").

For example, see a structure of the following two libraries `Foo` and `Bar`:

|--lib
|  |
|  |--communication
|  |  |--__init__.py
|  |  |--communication.py
|  |
|  |--gui
|  |  |--__init__.py
|  |  |--gui.py
|  |
|  |--session
|  |  |--__init__.py
|  |  |--session.py
|  |
|  |- README --> THIS FILE
|
|--src
   |-__init__.py
   |- main.py
