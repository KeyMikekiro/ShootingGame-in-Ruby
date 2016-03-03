class BulletType
    def initialize( image, speed, reload_time, status)
        @image = image
        @speed = speed
        @reload_time = reload_time
    end
    attr_reader :reload_time, :image, :speed
end

class Bullet < UnitObject
    def initialize( x, y, bullet_type)
        super( x, y, bullet_type.image, bullet_type.speed)
    end
end