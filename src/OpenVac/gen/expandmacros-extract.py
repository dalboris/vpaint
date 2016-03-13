
INPUT_DIR = 'src'
OUTPUT_DIR = 'out'

def isDefineDirective(line):
    line = line.lstrip()
    words = line.split(' ');
    if len(words) > 0 and words[0] == '#define':
        if len(words) > 1 and words[1].endswith('_H'):
            return False
        return True
    return False

# read concatenated
out = ''
with open(OUTPUT_DIR + '/' + 'concatenated.h','r') as f:
    isFirstContinuationLine = True
    isContinuationLine = False
    for line in f:
        line = line.rstrip() # remove trailing whitespace
        if isContinuationLine:
            if isFirstContinuationLine:
                isFirstContinuationLine = False
            else:
                out += '___NEWLINE___'
        if isDefineDirective(line):
            isFirstContinuationLine = True
        if isContinuationLine or isDefineDirective(line):
            out += line + '\n'
            if line.endswith('\\'):
                isContinuationLine = True
            else:
                isContinuationLine = False
    f.close()
            
f = open(OUTPUT_DIR + '/' + 'extracted.h','w')
f.write(out)
f.close()
