
INPUT_DIR = 'src'
OUTPUT_DIR = 'out'

def isMacroDirective(line):
    line = line.lstrip()
    words = line.split(' ');
    if len(words) > 0 and words[0].startswith('#'):
        return True
    else:
        return False

# read concatenated
out = ''
with open('temp.h','r') as f:
    isContinuationLine = False
    isFirstContinuationLine = True
    for line in f:
        line = line.rstrip() # remove trailing whitespace
        if isContinuationLine:
            out += '___NEWLINE___'
        if isContinuationLine or isMacroDirective(line):
            line = line.lstrip()
            isFirst = True
            for word in line.split(' '):
                if isFirst:
                    isFirst = False
                else:
                    out += ' '
                out += '___GUARD___' + word
            if line.endswith('\\'):
                isContinuationLine = True
                out += '___GUARD___'
            else:
                isContinuationLine = False
        else:
            out += line
        out += '\n'
    f.close()
            
f = open('temp2.h','w')
f.write(out)
f.close()
