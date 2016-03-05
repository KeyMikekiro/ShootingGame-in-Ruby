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

Encount_Time = 300
Re_Encount = 10

def init()
    GC.enable
    Audio.load( "./resource/music")
    @time_count = 0
    
    @game_start = false

    playerSpeed = { :x=>15, :y=>15}
    playerStatus = { :hp=>100, :attack=>10, :invincible_time=>60, :number_guns=>2}
    playerBullet = BulletType.new( Resource.image("player_bullet"), {:x=>0, :y=>-30}, 5, {:attack=>5})
    @player = Player.new( 500, 300, Resource.image("player_normal"), playerSpeed, playerStatus, playerBullet)
    
    @stage = TestStage.new( Re_Encount, Encount_Time)
end


def main
    init()
    Audio.play( 1, 0)
    Window.loop do
        GameWindow.draw_ui()
        GameWindow.debug()
        if !@game_start then
            @game_start = true if Input.key_release?(K_SPACE)
            GameWindow.draw_start_title
        else
            GameWindow.chengePause if Input.key_release?(K_ESCAPE)
            if !GameWindow.pause? then
                @stage.update()
                @stage.event()
            end
            @stage.draw()
            if !@player.dead? then
                if !GameWindow.pause? then
                    @player.damage( BulletManager.colision( @player, Player, BulletFlag::Enemy))
                    @player.update()
                    @player.input()
                end
                @player.draw()
            else
                GameWindow.draw_gameover()
            end
            @player.debug()
            @stage.enemies( @player)
            BulletManager.draw()
            BulletManager.update() if !GameWindow.pause?()
            Score.update()
            Score.draw()
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