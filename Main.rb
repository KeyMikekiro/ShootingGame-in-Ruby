#coding: utf-8
require 'dxruby'
require_relative 'ayame'
Window.width = 960
Window.height = 720

require './Resource'
Resource.init()
require './Audio'
require './Fonts'
require './Classes'
require './GameWindow'


def init()
    encount = 10
    re_encount_time = 15
    Audio.load( "./resource/music")
    
    @game_start = false

    playerSpeed = { :x=>5, :y=>5}
    playerStatus = { :hp=>100, :attack=>10}
    playerBullet = BulletType.new( Image.new( 8, 40, C_YELLOW), {:x=>0, :y=>-15}, 5, {:attack=>5})
    @player = Player.new( 500, 300, Image.new( 50, 50, C_RED), playerSpeed, playerStatus, playerBullet)
    
    enemySpeed = { :x=>0, :y=>1}
    enemyStatus = { :hp=>50, :attack=>5}
    @enemy = Enemy.new( 300, 0, Image.new( 50, 50, C_GREEN), enemySpeed, enemyStatus)
    @enemies = []
    for i in 0...encount do
        @enemies.push( Enemy.new( rand(300) + 100, 0, Image.new( 50, 50, C_GREEN), enemySpeed, enemyStatus.dup))
    end
end

def main
    init()
    Audio.play( 1, 0)
    Window.loop do
        GameWindow.draw_ui()
        GameWindow.debug_draw_font( GameWindow.height, 0, "enemy_count: " + @enemies.size.to_s, Fonts::Middle)
        if !@game_start then
            @game_start = true if Input.key_release?(K_SPACE)
            GameWindow.draw_start_title
        else
            @player.update()
            @player.input()
            @player.draw()
            @player.debug()
            for enemy in @enemies do
                enemy.update()
                enemy.draw()
                enemy.damage( @player.colision( enemy))
                @enemies.delete( enemy) if enemy.dead?
            end
        end
    end
end

main()