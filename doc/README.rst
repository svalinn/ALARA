Building the ALARA Documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ALARA documentation is built using the Sphinx_ documentation generator.

ALARA uses a two-branch system to maintain a clean process of rebuilding the
website. The ``main`` branch contains the source restructured text documents and
Sphinx configuration used to build the site. The ``gh-pages`` branch contains
the processed and published web content that is produced by Sphinx using the
source files in the ``main`` branch. The files in this branch should NOT be
edited directly.

The ALARA documentation build system relies on the ``doc.Makefile`` file located
in the top-level directory of the ALARA repository. Here is a summary of the
available commands:

``make -f doc.Makefile help``: Display available options and exit.

``make -f doc.Makefile html``: Build the documentation for viewing on a local
machine.

``make -f doc.Makefile clean``: Remove the locally-built documentation.

``make -f doc.Makefile publish``: Publish the documentation located in the
``main`` branch to the ``gh-pages`` branch. To prevent a situation where the
wrong branch is used to build the documentation, the git remote ``origin``
should be the main repository and not a fork. Additionally, the branch used for
building the documentation should not contain any additional changes not present
on Github. In other words, in order to use ``make publish``, the result of
``git remote -v && git status`` should be
::

    origin  https://github.com/svalinn/ALARA (fetch)
    origin  https://github.com/svalinn/ALARA (push)
    On branch main
    Your branch is up-to-date with 'origin/main'.
    nothing to commit, working tree clean

..  _Sphinx: https://www.sphinx-doc.org
