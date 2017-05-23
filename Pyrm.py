from flask import render_template
from gevent.wsgi import WSGIServer
import configparser, shutil
import os
from flask import Flask, request, redirect, url_for
from sys import platform as _platform

path = os.path.dirname(os.path.abspath(__file__))

# Config data
app = Flask(__name__)
global contador
# Caminho das Pastas
if _platform == "linux" or _platform == "linux2":  # linux
    projetos = path + "/Projects"
    script = path + '/script'
    loader = path + "/micropython/teensy/teensy_loader_cli"
    lang = path + '/lang.ini'
    conf = path + '/config.ini'
    teensy = path + '/micropython/teensy'
    config = configparser.ConfigParser()
elif _platform == "darwin":  # MAC OS X
    projetos = path + "/Projects"
    script = path + '/script'
    loader = path + "/micropython/teensy/teensy_loader_cli"
    lang = path + '/lang.ini'
    conf = path + '/config.ini'
    teensy = path + '/micropython/teensy'
    config = configparser.ConfigParser()
elif _platform == "win32":  # Windows
    projetos = path + "\Projects"
    script = path + '\script'
    loader = path + "\micropython\teensy\teensy_loader_cli"
    conf = path + '\config.ini'
    lang = path + '\lang.ini'
    teensy = path + '\micropython\teensy'
    config = configparser.ConfigParser()


contador = 0

@app.route("/")  # Welcome Screen
def index():
    config = configparser.ConfigParser()
    config.read(conf)
    language = config.get('DEFAULT', 'LANG')
    config.read(lang)
    welcome = config.get(language, 'welcome')
    welcome2 = config.get(language, 'welcome2')
    start = config.get(language, 'start')
    langIndex = request.args.get('lang', language, type=str)
    if langIndex == 'PT':
        writeConf('DEFAULT', 'LANG', 'PT')
    elif langIndex == 'EN':
        writeConf('DEFAULT', 'LANG', 'EN')
    return render_template("index.html", welcome=welcome, welcome2=welcome2, start=start)


@app.route("/board")  # Board Screen
def board():
    config = configparser.ConfigParser()
    config.read(conf)
    language = config.get('DEFAULT', 'LANG')
    config.read(lang)
    select = config.get(language, 'select')
    processor = config.get(language, 'processor')
    return render_template("board.html", select=select, processor=processor)


@app.route("/projeto", methods=['GET', 'POST'])  # Project Screen
def projeto():

    config = configparser.ConfigParser()
    config.read(conf)
    language = config.get('DEFAULT', 'LANG')
    config.read(lang)
    project = config.get(language, 'project')
    youselect = config.get(language, 'youselect')
    nameofproject = config.get(language, 'nameofproject')
    deleteproject = config.get(language, 'deleteproject')
    renameproject = config.get(language, 'renameproject')
    error = config.get(language, 'error')
    warning = config.get(language, 'warning')
    erroexists = config.get(language, 'erroexists')
    errodontexists = config.get(language, 'errodontexists')
    errorsomething = config.get(language, 'errorsomething')
    back = config.get(language, 'back')

    boardName = 'Placa nao encontrada'
    erro = None
    board = request.args.get('board', 'mk66fx1m0', type=str)
    newproject = request.args.get('newproject', '', type=str)
    delete = request.args.get('delete', '', type=str)
    edit = request.args.get('edit', '', type=str)
    verifConf()
    writeConf('DEFAULT', 'board', board)
    if board == 'mk64fx512':
        boardName = "Teensy 3.5"
    elif board == 'mk66fx1m0':
        boardName = 'Teensy 3.6'
    elif board == 'mk20dx256':
        boardName = 'Teensy 3.1'
    pastas = []
    if newproject != '':
        if newproject != 'null':
            if not os.path.exists(projetos + '/' + newproject):
                os.makedirs(projetos + '/' + newproject)
                shutil.copyfile(script + '/main.py', projetos + '/' + newproject + '/main.py')
                shutil.copyfile(script + '/boot.py', projetos + '/' + newproject + '/boot.py')
            else:
                erro = erroexists
    if delete != '':
        if delete != 'null':
            if os.path.exists(projetos + '/' + delete):
                shutil.rmtree(projetos + '/' + delete)
            else:
                erro = errodontexists
    if edit != '':
        if edit != 'null':
            try:
                old, new = edit.split(',')

                if os.path.exists(projetos + '/' + old):
                    os.renames(projetos + '/' + old, projetos + '/' + new)
                else:
                    erro = errodontexists
            except:
                erro = errorsomething
    for filename in os.listdir(projetos):
        pastas.append(filename)

    return render_template("projeto.html", board=boardName, pastas=pastas, erro=erro, error=error, warning=warning,
                           project=project, youselect=youselect, nameofproject=nameofproject,
                           deleteproject=deleteproject, renameproject=renameproject, back=back)


@app.route("/dev", methods=['GET', 'POST'])  # Board Screen
def dev():
    global contador
    config = configparser.ConfigParser()
    config.read(conf)
    language = config.get('DEFAULT', 'LANG')
    config.read(lang)
    editor = config.get(language, 'editor')
    back = config.get(language, 'back')
    error = config.get(language, 'error')
    warning = config.get(language, 'warning')
    filesaved = config.get(language, 'filesaved')
    flashok = config.get(language, 'flashok')

    boardok = config.get(language, 'boardok')

    projetoStr = request.args.get('projeto', '', type=str)
    file = request.args.get('file', '', type=str)
    flash = request.args.get('flash', False, type=bool)
    aviso = None
    if projetoStr == '':
        return redirect(url_for("projeto"))
    if file != '':
        arquivo = open(projetos + '/' + projetoStr + '/' + file, 'r')
        arquivo = arquivo.read()

        if request.method == 'POST':
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'w')
            code = request.form["code"]
            arquivo.write(code)
            arquivo.close()
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'r')
            arquivo = arquivo.read()
            aviso = filesaved
            flash = False

        if flash:
            config.read(conf)
            board = config.get(language, 'board')

            if board == 'mk66fx1m0':
                from subprocess import Popen, PIPE
                print teensy + 'scripts/main.py'
                print projetos + '/' + projetoStr + '/main.py'
                shutil.copyfile(projetos + '/' + projetoStr + '/main.py', teensy + '/scripts/main.py')
                shutil.copyfile(projetos + '/' + projetoStr + '/boot.py', teensy + '/scripts/boot.py')
                cmd = 'make --directory ' + teensy + ' BOARD=TEENSY_3.6'
                print cmd
                proc = Popen(cmd, shell=True, bufsize=1, stdout=PIPE)

                # ./teensy_loader_cli --mcu=mk66fx1m0 -w micropython.hex
                cmd2 =loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.6/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                contador = contador + 1
                if contador == 1:
                    aviso = boardok
                elif contador == 2:
                    aviso = flashok
                    contador = 0
            elif board == 'mk64fx512':
                from subprocess import Popen, PIPE
                print teensy + 'scripts/main.py'
                print projetos + '/' + projetoStr + '/main.py'
                shutil.copyfile(projetos + '/' + projetoStr + '/main.py', teensy + '/scripts/main.py')
                shutil.copyfile(projetos + '/' + projetoStr + '/boot.py', teensy + '/scripts/boot.py')
                cmd = 'make --directory ' + teensy + ' BOARD=TEENSY_3.6'
                print cmd
                proc = Popen(cmd, shell=True, bufsize=1, stdout=PIPE)

                # ./teensy_loader_cli --mcu=mk64fx512 -w micropython.hex
                cmd2 =loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.6/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                contador = contador + 1
                if contador == 1:
                    aviso = boardok
                elif contador == 2:
                    aviso = flashok
                    contador = 0
            elif board == 'mk20dx256':
                from subprocess import Popen, PIPE
                print teensy + 'scripts/main.py'
                print projetos + '/' + projetoStr + '/main.py'
                shutil.copyfile(projetos + '/' + projetoStr + '/main.py', teensy + '/scripts/main.py')
                shutil.copyfile(projetos + '/' + projetoStr + '/boot.py', teensy + '/scripts/boot.py')
                cmd = 'make --directory ' + teensy + ' BOARD=TEENSY_3.6'
                print cmd
                proc = Popen(cmd, shell=True, bufsize=1, stdout=PIPE)

                # ./teensy_loader_cli --mcu=mk20dx256 -w micropython.hex
                cmd2 = loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.6/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                contador = contador + 1
                if contador == 1:
                    aviso = boardok
                elif contador == 2:
                    aviso = flashok
                    contador = 0
        erro = None
        arquivos = []
        for filename in os.listdir(projetos + "/" + projetoStr):
            arquivos.append(filename)

        return render_template("dev.html", projeto=projetoStr, arquivo=arquivo, file=file, erro=erro, arquivos=arquivos,
                               aviso=aviso, warning=warning, error=error, back=back, editor=editor)
    else:
        return redirect(url_for("projeto"))


def writeConf(section='DEFAULT', nome='nome', valor='NO'):  # funcao para escrever no arquivo conf.ini
    config.read(conf)
    config[section][nome] = valor
    with open(conf, 'w') as configfile:
        config.write(configfile)


def verifConf():  # verifica se existe o arquivo conf.ini, se nao tiver ele cria
    try:
        with open(conf, 'r') as f:
            return 0
    except IOError:
        config['DEFAULT'] = {'board': 'mk66fx1m0', 'lang': 'EN'}
        with open(conf, 'w') as configfile:
            config.write(configfile)
        return 1


if __name__ == '__main__':
    http_server = WSGIServer(('0.0.0.0', 1515), app)
    http_server.serve_forever()
