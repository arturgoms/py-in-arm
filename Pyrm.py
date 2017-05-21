from flask import Flask, request, url_for, redirect
from flask import render_template
from gevent.wsgi import WSGIServer
import configparser, shutil
import os
from flask import Flask, request, redirect, url_for
from werkzeug.utils import secure_filename

UPLOAD_FOLDER = '/Volumes/Armazenamento/Projetos/Pyrm/Projects'

# Config data
INI = [True, "0.0.0.0", 8080]
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
app = Flask(__name__)


# Caminho das Pastas
projetos = "/Volumes/Armazenamento/Projetos/Pyrm/Projects"
script = '/Volumes/Armazenamento/Projetos/Pyrm/script'
loader = "teensy/teensy_loader_cli"
conf = "config.ini"
config = configparser.ConfigParser()


@app.route("/")  # Welcome Screen
def index():
    return render_template("index.html")


@app.route("/board")  # Board Screen
def board():
    return render_template("board.html")


@app.route("/projeto", methods=['GET', 'POST'])  # Project Screen
def projeto():
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
                erro = 'Projeto ja existe'
    if delete != '':
        if delete != 'null':
            if os.path.exists(projetos + '/' + delete):
                shutil.rmtree(projetos + '/' + delete)
            else:
                erro = 'Projeto nao existe'
    if edit != '':
        if edit != 'null':
            try:
                old, new = edit.split(',')

                if os.path.exists(projetos + '/' + old):
                    os.renames(projetos + '/' + old, projetos + '/' + new)
                else:
                    erro = 'Projeto nao existe'
            except:
                erro = 'Algo de errado aconteceu.'
    for filename in os.listdir(projetos):
        pastas.append(filename)


    return render_template("projeto.html", board=boardName, pastas=pastas, erro=erro)


@app.route("/dev", methods=['GET', 'POST'])  # Board Screen
def dev():
    projetoStr = request.args.get('projeto', '', type=str)
    file = request.args.get('file', '', type=str)
    flash = request.args.get('flash', False, type=bool)
    aviso = None
    if projetoStr == '':
        return redirect(url_for("projeto"))
    if file != '':
        app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER + '/' + projetoStr
        arquivo = open(projetos + '/' + projetoStr + '/' + file, 'r')
        arquivo = arquivo.read()

        if request.method == 'POST':
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'w')
            code = request.form["code"]
            arquivo.write(code)
            arquivo.close()
            arquivo = open(projetos + '/' + projetoStr + '/' + file, 'r')
            arquivo = arquivo.read()
            aviso = 'Arquivo salvo'
            flash = False

        if flash:
            aviso = 'Flash concluido'
        erro = None
        arquivos = []
        for filename in os.listdir(projetos + "/" + projetoStr):
            arquivos.append(filename)


        return render_template("dev.html", projeto=projetoStr, arquivo=arquivo, file=file, erro=erro, arquivos=arquivos,
                               aviso=aviso)
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
        config['DEFAULT'] = {'board': 'mk66fx1m0'}

        with open(conf, 'w') as configfile:
            config.write(configfile)
        return 1


if __name__ == '__main__':
    http_server = WSGIServer(('', INI[2]), app)
    http_server.serve_forever()
