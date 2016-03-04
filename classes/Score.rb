module Score
    Font = Fonts::Large
    String = "Score: "
    @@ScoreDatas = {}
    @@point = 0
    
    def self.load
    end
    
    def self.save
    end
    
    def self.add_point( point)
        @@point += point
    end
    
    def self.get()
        return @@point
    end
    
    def self.update()
    end
    
    def self.draw()
        GameWindow.draw_font( 0, Font.size, String + @@point.to_s, Font)
    end
    
    def self.show_result()
    end
end