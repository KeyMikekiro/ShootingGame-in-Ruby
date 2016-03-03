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
    GC.enable
    encount = 10
    re_encount_time = 15
    Audio.load( "./resource/music")
    @time_count = 0
    
    @game_start = false

    playerSpeed = { :x=>15, :y=>15}
    playerStatus = { :hp=>100, :attack=>10, :invincible_time=>60}
    playerBullet = BulletType.new( Image.new( 8, 40, C_YELLOW), {:x=>0, :y=>-30}, 5, {:attack=>5})
    @player = Player.new( 500, 300, Resource.image("player_normal"), playerSpeed, playerStatus, playerBullet)
    
    enemySpeed = { :x=>0, :y=>1}
    enemyStatus = { :hp=>50, :attack=>5}
    @enemy = Enemy.new( 300, 0, Resource.image("enemy"), enemySpeed, enemyStatus)
    @enemies = []
    for i in 0...encount do
        @enemies.push( Enemy.new( rand(300) + 100, 0, Resource.image("enemy"), enemySpeed, enemyStatus.dup))
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