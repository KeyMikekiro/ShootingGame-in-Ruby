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
    playerStatus = { :hp=>100, :attack=>10, :invincible_time=>60, :number_guns=>2}
    playerBullet = BulletType.new( Resource.image("player_bullet"), {:x=>0, :y=>-30}, 5, {:attack=>5})
    @player = Player.new( 500, 300, Resource.image("player_normal"), playerSpeed, playerStatus, playerBullet)
    
    enemySpeed = { :x=>0, :y=>1}
    enemyStatus = { :hp=>50, :attack=>5}
    enemyBullet = BulletType.new( Resource.image("enemy_bullet"), {:x=>rand(3)-1, :y=>15}, 30, {:attack=>1})
    @enemy = Enemy.new( 300, 0, Resource.image("enemy"), enemySpeed, enemyStatus)
    @enemies = []
    for i in 0...encount do
        @enemies.push( Enemy.new( rand(300) + GameWindow.x, 0,
            Resource.image("enemy"), enemySpeed, enemyStatus.dup, enemyBullet))
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
            GameWindow.chengePause if Input.key_release?(K_ESCAPE)
            if !GameWindow.pause? then
                @player.damage( BulletManager.colision( @player, Player, BulletFlag::Enemy))
                @player.update()
                @player.input()
            end
            @player.draw()
            @player.debug()
            for enemy in @enemies do
                if !GameWindow.pause? then
                    enemy.update()
                    enemy.damage( BulletManager.colision( enemy, Enemy, BulletFlag::Player))
                    enemy.damage( @player.colision( enemy))
                end
                @enemies.delete( enemy) if enemy.dead?
                enemy.draw()
            end
            BulletManager.draw()
            BulletManager.update()
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