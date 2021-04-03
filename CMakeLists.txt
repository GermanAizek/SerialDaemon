cmake_minimum_required(VERSION 3.5)

project(serialdaemon LANGUAGES C)

add_executable(serialdaemon seriallogger.c seriallogger.h util.c util.h)
