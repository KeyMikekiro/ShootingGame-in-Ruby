module BulletFlag
    Player = :player
    Enemy = :enemy
end

class BulletType
    def initialize( image, speed, reload_time, status)
        @image = image
        @speed = speed
        @reload_time = reload_time
        @status = status
    end
    attr_reader :reload_time, :image, :speed, :status
end

class Bullet < UnitObject
    def initialize( x, y, bullet_type)
        x -= bullet_type.image.width / 2
        super( x, y, bullet_type.image, bullet_type.speed)
        @status = bullet_type.status
    end
    
    def draw()
        GameWindow.draw_sprite( @sprite, GameWindow::Game_draw_order - 5)
    end
    attr_reader :status
end