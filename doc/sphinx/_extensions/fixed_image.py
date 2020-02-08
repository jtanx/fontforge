import docutils.nodes

def setup(app):
    '''
    Massive hack around https://github.com/sphinx-doc/sphinx/issues/7112
    app.add_node is not sufficient, as docutils directly imports and uses
    its image class directly, so resort to some more aggressive methods...
    '''

    class SpoofStr(str):
        def lower(self):
            return '-spoofed-str-'

    orig_getitem = docutils.nodes.image.__getitem__
    def patched(self, key):
        val = orig_getitem(self, key)
        if key == 'uri':
            return SpoofStr(val)
        return val
    docutils.nodes.image.__getitem__ = patched

    return {
        'version': '1.0.0',
        'env_version': 1,
        'parallel_read_safe': True
    }
