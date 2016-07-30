class GameAdmin
    def initialize()
        @game_start = false

        playerSpeed = { :x=>15, :y=>15}
        playerStatus = { :hp=>100, :attack=>10, :invincible_time=>60, :number_guns=>2}
        playerBullet = BulletType.new( Resource.image("player_bullet"), {:x=>0, :y=>-30}, 5, {:attack=>5})
        @player = Player.new( 500, 300, Resource.image("player_normal"), playerSpeed, playerStatus, playerBullet)
        
        @stage = TestStage.new( 10, 30)
    end

    def gameRun()
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
        end
        @player.debug()
        @stage.enemies( @player)
        BulletManager.draw()
        BulletManager.update() if !GameWindow.pause?()
        Score.update()
        Score.draw()
    end

    def playerDead?()
        return @player.dead?
    end
end
