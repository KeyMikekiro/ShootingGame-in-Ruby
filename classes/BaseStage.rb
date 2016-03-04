class BaseStage
    Star_size = 2
    Stars_image = [
        Image.new( Star_size, Star_size, [255, 255, 0, 0]), Image.new( Star_size, Star_size, [255, 0, 255, 0]),
        Image.new( Star_size, Star_size, [255, 0, 0, 255]), Image.new( Star_size, Star_size, [255, 255, 255, 255])
    ]
    
    class Star < UnitObject
        def initialize( x, y, image, speed)
            super( x, y, image, speed)
        end
    end
    
    def initialize( encount_enemy_num=0, encount_time=0, mapdata=nil)
        @encount_enemy_num = encount_enemy_num
        @encount_time = encount_time
        @mapdata = mapdata
        @count = 0
        @stage_move_speed = {:x=>0, :y=>5}
        @stars = star_dust_shack_hand( 45, Window.height)
    end
    
    def event
    end
    
    def update
        if @count%10 == 0 then
            @count = 0
            @stars.concat(star_dust_shack_hand( 5, 20))
        end
        for star in @stars do
            star.update()
        end
        @count += 1
    end
    
    def draw
        for star in @stars do
            GameWindow.draw_sprite( star.sprite)
        end
    end
    
    def star_dust_shack_hand( random_num, y)
        stars = []
        for index in 0..rand(random_num)+5 do
            stars.push( Star.new( rand(GameWindow.width) + GameWindow.x,
                rand( y), Stars_image[rand(Stars_image.size)], @stage_move_speed))
        end
        return stars
    end
end