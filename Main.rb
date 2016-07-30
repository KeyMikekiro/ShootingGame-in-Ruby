#coding: utf-8
require 'dxruby'
require_relative 'ayame'
Window.width = 960
Window.height = 720

require './Resource'
Resource.init()
require './Audio'
require './Fonts'
require './GameWindow'
require './Classes'
require './Datas'

module GameSince
    Start = :start
    GameMain = :gameMain
    Result = :result
end

def init()
    GC.enable
    Audio.load( "./resource/music")
    @time_count = 0
    @game_since = GameSince::Start
    @game_admin = GameAdmin.new()
end


def main
    init()
    Audio.play( 1, 0)
    Window.loop do
        GameWindow.draw_ui()
        GameWindow.debug()
        case @game_since
        when GameSince::Start then
            @game_since = GameSince::GameMain if Input.key_release?(K_SPACE)
            GameWindow.draw_start_title()
        when GameSince::GameMain then
            @game_since = GameSince::Result if @game_admin.playerDead?
            @game_admin.gameRun()
        when GameSince::Result then
            GameWindow.draw_gameover()
        end
        if @time_count > 60 then
            @time_count = 0
            GC.disable
            GC.start
            GC.enable
        end
        @time_count += 1
    end
end

main()