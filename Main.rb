#coding: utf-8
require 'dxruby'
Window.width = 960
Window.height = 720

require './Fonts'
require './UnitObject'
require './Player'
require './Bullet'

class Enemy < UnitObject
    def initialize( x, y, image, speed, status)
        super( x, y, image, speed)
        @status = status
    end
    attr_reader :status
    
    def damage( unit)
        @status[:hp] -= unit.status[:attack] if unit != nil
    end
    
    def draw
        super
        Window.draw_font( @sprite.x, @sprite.y, @status[:hp].to_s, Fonts::Middle)
    end
    
    def dead?
        return @status[:hp] <= 0
    end
end

def init()
    encount = 10
    re_encount_time = 15

    playerSpeed = { :x=>5, :y=>5}
    playerStatus = { :hp=>100, :attack=>10}
    playerBullet = BulletType.new( Image.new( 8, 40, C_YELLOW), {:x=>0, :y=>-15}, 5)
    @player = Player.new( 0, 0, Image.new( 50, 50, C_RED), playerSpeed, playerStatus, playerBullet)
    
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
    Window.loop do
        @player.update()
        @player.input()
        @player.draw()
        @player.debug()
        for enemy in @enemies do
            enemy.update()
            enemy.draw()
            enemy.damage( @player.colision)
            @enemies.delete( enemy) if enemy.dead?
        end
    end
end

main()