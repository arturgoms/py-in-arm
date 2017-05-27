from flask import render_template, send_file, send_from_directory
from flask_images import Images, resized_img_src
from gevent.wsgi import WSGIServer
import configparser, shutil
import os, zipfile
from flask import Flask, request, redirect, url_for
from sys import platform as _platform

path = os.path.dirname(os.path.abspath(__file__))

# Config data
app = Flask(__name__)
images = Images(app)
global contador
# Caminho das Pastas
if _platform == "linux" or _platform == "linux2":  # linux
    projetos = path + "/Projects"
    zip = path + "/zip"
    script = path + '/script'
    loader = path + "/micropython/teensy/teensy_loader_cli"
    lang = path + '/lang.ini'
    conf = path + '/config.ini'
    teensy = path + '/micropython/teensy'
    config = configparser.ConfigParser()
elif _platform == "darwin":  # MAC OS X
    projetos = path + "/Projects"
    zip = path + "/zip"
    script = path + '/script'
    loader = path + "/micropython/teensy/teensy_loader_cli"
    lang = path + '/lang.ini'
    conf = path + '/config.ini'
    teensy = path + '/micropython/teensy'
    config = configparser.ConfigParser()
elif _platform == "win32":  # Windows
    projetos = path + "\Projects"
    zip = path + "\zip"
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
                #os.remove(projetos + '/' + delete)
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
    erroexists = config.get(language, 'erroexists')
    errodontexists = config.get(language, 'errodontexists')
    errorsomething = config.get(language, 'errorsomething')
    boardok = config.get(language, 'boardok')

    projetoStr = request.args.get('projeto', '', type=str)
    file = request.args.get('file', '', type=str)
    folder = request.args.get('folder', '', type=str)
    flash = request.args.get('flash', False, type=bool)
    zipfiles = request.args.get('zip', False, type=bool)
    buildHex = request.args.get('build', False, type=bool)
    newfile = request.args.get('newfile', '', type=str)
    deletefile = request.args.get('deletefile', '', type=str)
    editfile = request.args.get('editfile', '', type=str)
    newfolder = request.args.get('newfolder', '', type=str)
    deletefolder = request.args.get('deletefolder', '', type=str)
    editfolder = request.args.get('editfolder', '', type=str)

    aviso = None
    erro = None

    if file != '':
        try:
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'r')
            arquivo = arquivo.read()
        except:
            arquivo = 'Selecione um arquivo'
    else:
        arquivo = 'Selecione um arquivo'
    if request.method == 'POST':
        if os.path.exists(projetos + '/' + projetoStr + '/' + file):
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'w')
            code = request.form["code"]
            arquivo.write(code)
            arquivo.close()
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'r')
            arquivo = arquivo.read()
            aviso = filesaved
            flash = False
        else:
            aviso = 'Erro saving file'
            flash = False


    if newfolder != '':
        if newfolder != 'null':
            if not os.path.exists(projetos + '/' + projetoStr + '/' + newfolder):
                os.makedirs(projetos + '/' + projetoStr + '/' + newfolder)
            else:
                erro = erroexists
    if deletefolder != '':
        if deletefolder != 'null':
            if os.path.exists(projetos + '/' + projetoStr + '/' + deletefolder):
                shutil.rmtree(projetos + '/' + projetoStr + '/' + deletefolder)
            else:
                erro = errodontexists
    if editfolder != '':
        if editfolder != 'null':
            try:
                old, new = editfolder.split(',')

                if os.path.exists(projetos + '/' + projetoStr + '/' + old):
                    os.renames(projetos + '/' + projetoStr + '/' + old, projetos + '/' + projetoStr + '/' + new)
                else:
                    erro = errodontexists
            except:
                erro = errorsomething
    if newfile != '':
        if newfile != 'null':
            if not os.path.exists(projetos + '/' + projetoStr + '/' + newfile):
                open(projetos + '/' + projetoStr + '/' + newfile, 'w')
            else:
                erro = erroexists
    if deletefile != '':
        if deletefile != 'null':
            if os.path.exists(projetos + '/' + projetoStr + '/' + deletefile):
                os.remove(projetos + '/' + projetoStr + '/' + deletefile)
            else:
                erro = errodontexists
    if editfile != '':
        if editfile != 'null':
            try:
                old, new = editfile.split(',')

                if os.path.exists(projetos + '/' + projetoStr + '/' + old):
                    os.renames(projetos + '/' + projetoStr + '/' + old, projetos + '/' + projetoStr + '/' + new)
                else:
                    erro = errodontexists
            except:
                erro = errorsomething
    if flash:
        config.read(conf)
        board = config.get(language, 'board')

        if board == 'mk66fx1m0':
            from subprocess import Popen, PIPE
            print teensy + 'scripts/main.py'
            print projetos + '/' + projetoStr + '/main.py'
            folder = teensy + '/scripts/'
            for the_file in os.listdir(folder):
                file_path = os.path.join(folder, the_file)
                try:
                    if os.path.isfile(file_path):
                        os.unlink(file_path)
                        # elif os.path.isdir(file_path): shutil.rmtree(file_path)
                except Exception as e:
                    print(e)
            src_files = os.listdir(projetos + '/' + projetoStr)
            for file_name in src_files:
                full_file_name = os.path.join(projetos + '/' + projetoStr, file_name)
                if (os.path.isfile(full_file_name)):
                    shutil.copy(full_file_name, teensy + '/scripts')

            if buildHex:
                try:
                    os.popen('make --directory ' + teensy + ' BOARD=TEENSY_3.6 clean', 'w')
                    cmd = 'make --directory ' + teensy + ' BOARD=TEENSY_3.6'
                    print cmd
                    os.popen(cmd, 'w')
                    aviso = "Build finish"
                except:
                    erro = 'Build failed'
            else:
                try:
                    cmd2 = loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.6/micropython.hex'
                    print cmd2
                    os.popen(cmd2, 'w')
                    contador = contador + 1
                    if contador == 1:
                        aviso = boardok
                    elif contador == 2:
                        aviso = flashok
                except:
                    erro = 'Flash failed'
        elif board == 'mk64fx512':
            from subprocess import Popen, PIPE
            print teensy + 'scripts/main.py'
            print projetos + '/' + projetoStr + '/main.py'
            folder = teensy + '/scripts/'
            for the_file in os.listdir(folder):
                file_path = os.path.join(folder, the_file)
                try:
                    if os.path.isfile(file_path):
                        os.unlink(file_path)
                        # elif os.path.isdir(file_path): shutil.rmtree(file_path)
                except Exception as e:
                    print(e)
            src_files = os.listdir(projetos + '/' + projetoStr)
            for file_name in src_files:
                full_file_name = os.path.join(projetos + '/' + projetoStr, file_name)
                if (os.path.isfile(full_file_name)):
                    shutil.copy(full_file_name, teensy + '/scripts')
            contador = contador + 1
            if contador == 1:
                os.popen('make --directory ' + teensy + ' BOARD=TEENSY_3.5 clean', 'w')
                cmd = 'make --directory ' + teensy + ' BOARD=TEENSY_3.5'
                print cmd
                os.popen(cmd, 'w')
                aviso = "Build finish"
            elif contador == 2:
                cmd2 = loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.5/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                aviso = boardok
            elif contador == 3:
                cmd2 = loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.5/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                aviso = flashok
                contador = 0
        elif board == 'mk20dx256':
            from subprocess import Popen, PIPE
            print teensy + 'scripts/main.py'
            print projetos + '/' + projetoStr + '/main.py'
            folder = teensy + '/scripts/'
            for the_file in os.listdir(folder):
                file_path = os.path.join(folder, the_file)
                try:
                    if os.path.isfile(file_path):
                        os.unlink(file_path)
                        # elif os.path.isdir(file_path): shutil.rmtree(file_path)
                except Exception as e:
                    print(e)
            src_files = os.listdir(projetos + '/' + projetoStr)
            for file_name in src_files:
                full_file_name = os.path.join(projetos + '/' + projetoStr, file_name)
                if (os.path.isfile(full_file_name)):
                    shutil.copy(full_file_name, teensy + '/scripts')
            contador = contador + 1
            if contador == 1:
                os.popen('make --directory ' + teensy + ' BOARD=TEENSY_3.1 clean', 'w')
                cmd = 'make --directory ' + teensy + ' BOARD=TEENSY_3.1'
                print cmd
                os.popen(cmd, 'w')
                aviso = "Build finish"
            elif contador == 2:
                cmd2 = loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.1/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                aviso = boardok
            elif contador == 3:
                cmd2 = loader + ' --mcu=' + board + ' -w ' + teensy + '/build-TEENSY_3.1/micropython.hex'
                print cmd2
                os.popen(cmd2, 'w')
                aviso = flashok
                contador = 0

    if zipfiles:
        zipfiles = False
        zipf = zipfile.ZipFile(zip + '/' + projetoStr+'.zip', 'w', zipfile.ZIP_DEFLATED)
        zipdir(zip, zipf)
        zipf.close()
        warning = "File Downloaded"
        return send_from_directory(directory=zip, filename=projetoStr+'.zip', as_attachment=True)

    arquivos = []
    pastas = []
    if projetoStr != '':
        for filename in os.listdir(projetos + "/" + projetoStr):
            w = os.path.isdir(projetos + "/" + projetoStr + "/" + filename)
            if w:
                pastas.append(filename)
            else:
                arquivos.append(filename)
    else:
        return redirect(url_for("projeto"))


    return render_template("dev.html", projeto=projetoStr, arquivo=arquivo, file=file, erro=erro, arquivos=arquivos, pastas=pastas,
                           aviso=aviso, warning=warning, error=error, back=back, editor=editor)



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


def zipdir(path, ziph):
    for root, dirs, files in os.walk(path):
        for file in files:
            ziph.write(os.path.join(root, file))


if __name__ == '__main__':
    http_server = WSGIServer(('0.0.0.0', 8080), app)
    http_server.serve_forever()
