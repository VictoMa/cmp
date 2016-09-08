#!usr/env/python
# -*- coding: UTF-8 -*-
import sys,os
def Compile():
	os.system("./compiler < test.mt")
def Edit():
	os.system("gedit ./test.mt")
def Run():
	os.system("./a.out")

import Tkinter


top = Tkinter.Tk()
top.title("MT Complier")
top.resizable(width=False, height=False)

Tkinter.Label(top,text="Welcome to MT Complier 1.0").grid(row=0,column=0,columnspan=6)
Tkinter.Button(top, text="Compile", command=Compile).grid(row=1,column=0,columnspan=2)
Tkinter.Button(top, text="Edit", command=Edit).grid(row=1,column=2,columnspan=2)
Tkinter.Button(top, text="Run", command=Run).grid(row=1,column=4,columnspan=2)


top.mainloop()



