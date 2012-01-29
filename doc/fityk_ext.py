
import re

from pygments.lexer import RegexLexer
from pygments.token import Text, Name, Comment, String, Generic
from sphinx import addnodes
from docutils import nodes

class FitykLexer(RegexLexer):
    name = 'fityklexer'

    tokens = {
            'root': [
                (r"'[^']*'", String.Single),
                (r'[#].*?$', Comment),
                (r'^=-> ', Generic.Prompt),
                (r"[^'#\n]+", Text),
                ],
            }

comment_re = re.compile(r'(\(\*.*?\*\))')

def doctree_read(app, doctree):
    env = app.builder.env
    for node in doctree.traverse(addnodes.productionlist):
        for production in node:
            if not isinstance(production, addnodes.production):
                continue
            if not isinstance(production[-1], nodes.Text):
                continue
            parts = comment_re.split(production.pop().astext())
            new_nodes = []
            for s in parts:
                if comment_re.match(s):
                    new_nodes.append(nodes.emphasis(s, s))
                elif s:
                    new_nodes.append(nodes.Text(s))
            production += new_nodes


def setup(app):
    app.add_lexer('fityk', FitykLexer());
    app.connect('doctree-read', doctree_read)



# this is hack is needed to use our layout.html on ReadTheDocs
from sphinx.jinja2glue import BuiltinTemplateLoader
from jinja2 import TemplateNotFound
class MyTemplateLoader(BuiltinTemplateLoader):
    def get_source(self, environment, template):
        # If template name in Jinja's "extends" is prepended with "!"
        # Sphinx skips project's template paths.
        # In BuiltinTemplateLoader self.templatepathlen is used to remove
        # project's template paths and leave only Sphinx's paths.
        # This hack should leave the last path, so "!layout.html" will find
        # the template from Fityk. To avoid recursion, Fityk template
        # is not using "!".
        print("\n(MyTemplateLoader.get_source) searching for %s" % template)
        loaders = self.loaders
        # exclamation mark starts search from theme
        if template.startswith('!'):
            loaders = loaders[self.templatepathlen-1:]
            template = template[1:]
        for loader in loaders:
            print("\ttrying in: %s" % ":".join(loader.searchpath))
            try:
                return loader.get_source(environment, template)
            except TemplateNotFound:
                pass
        raise TemplateNotFound(template)
