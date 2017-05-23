

<h1 align="center">
  <br>
  <a href="http://www.amitmerchant.com/electron-markdownify"><img src="https://dl.dropboxusercontent.com/s/uylcf1b53pxcwik/Hexagon.png?dl=0" alt="Markdownify" width="200"></a>
  <br>
</h1>

<h4 align="center">Python IDE for ARM Processors based on<a href="https://github.com/micropython/micropython" target="_blank"> Micropython </a>framework.</h4>

<p align="center">
  <a href="https://github.com/arturgoms/Pyrm">
    <img src="https://img.shields.io/badge/IDE-Python-brightgreen.svg" alt="Electron">
  </a>
  <a href="https://github.com/arturgoms/Pyrm/wiki">
    <img src="https://img.shields.io/badge/release-82%25-green.svg"
         alt="Gitter">
  </a>
  <a href="https://www.python.org/download/releases/2.7/"><img src="https://img.shields.io/badge/python-2.7-blue.svg"></a>

  <a href="https://www.paypal.me/t">
    <img src="https://img.shields.io/badge/$-donate-ff69b4.svg?maxAge=2592000&amp;style=flat">
  </a>
</p>
<br>

<p align="center">
 <img src="https://dl.dropboxusercontent.com/s/3os488hqdagf77b/Screen%20Shot%202017-05-22%20at%2016.49.51.png?dl=0" alt="SDVersion"/>
</p>



## How it works

It is an IDE created in Flask and based on the microPython framwork, everything was done in html and css, some parts in javascript and it was also used the library Codemirror for the syntax in the python editor.

### Support ( for now )

	Teensy 3.1
    Teensy 3.5
    Teensy 3.6

### Manual installation


 First you need to clone the directory:
 ```bash
      git clone https://github.com/arturgoms/Pyrm.git
 ```
The next step is install all the dependencies( if you already have pip installed):
 ```bash
      pip install -U -r  requirements.txt
 ```
If dont ( Windows ):
 ```bash
      python get-pip.py
 ```
(macOs):

 ```bash
      sudo easy_install pip
 ```
 After enter the directory you can now execute Pyrm.py:
 ```bash
      sudo python Pyrm.py
 ```


## License
Usage is provided under the [MIT License](http://opensource.org/licenses/mit-license.php). See LICENSE for the full details.
