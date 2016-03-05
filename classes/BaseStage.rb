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
        
        @enemies = []
        encount_enemy()
        @re_encount_time = 0
        
        @event_enemies = []
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
        
        for event_enemy in @event_enemies do
            event_enemy.update()
        end
        
        re_encount_enemy()
    end
    
    def enemies( player)
        for enemy in @enemies do
            if !GameWindow.pause? then
                enemy.damage( BulletManager.colision( enemy, Enemy, BulletFlag::Player))
                enemy.damage( player.colision( enemy))
                enemy.update()
                Score.add_point( enemy.status[:point]) if enemy.dead?
            end
            @enemies.delete( enemy) if enemy.dead?
            enemy.draw()
        end
        
        for event_enemy in @event_enemies do
            if !GameWindow.pause? then
                event_enemy.damage( BulletManager.colision( event_enemy, Enemy, BulletFlag::Player))
                event_enemy.damage( player.colision( event_enemy))
                event_enemy.update()
                Score.add_point( event_enemy.status[:point]) if event_enemy.dead?
            end
            @event_enemies.delete( event_enemy) if event_enemy.dead?
            event_enemy.draw()
        end
    end
    
    
    def encount_enemy()
        enemySpeed = { :x=>0, :y=>1}
        enemyStatus = { :hp=>50, :attack=>5, :point=>5}
        enemyBullet = BulletType.new( Resource.image("enemy_bullet"), {:x=>0, :y=>15}, 30, {:attack=>1})
        @enemy = Enemy.new( 300, 0, Resource.image("enemy"), enemySpeed, enemyStatus)
        for i in 0...@encount_enemy_num do
            @enemies.push( Enemy.new( rand(300) + GameWindow.x, 0,
                Resource.image("enemy"), enemySpeed, enemyStatus, enemyBullet))
        end
    end

    def re_encount_enemy()
        if @re_encount_time > @encount_time then
            
            encount_enemy()
            @re_encount_time = 0
        end
        @re_encount_time += 1
    end
    
    def draw
        for star in @stars do
            GameWindow.draw_sprite( star.sprite)
        end
        Sprite.clean( @stars)
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