class MiddleBoss < Enemy
    def moving_pattern()
        if Window.height / 2 < @sprite.y then
            @speed[:y] = 0
        end
    end
    
    
    def update()
        moving_pattern()
        super()
        action()
    end
    
    def action()
        GameWindow.draw_font( position_width, @sprite.y, "Test Test.", Fonts::Middle) if @status[:hp] < 50
    end
end
