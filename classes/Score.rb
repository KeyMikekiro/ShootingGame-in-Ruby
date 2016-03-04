module Score
    Font = Fonts::Large
    String = "Score: "
    @@ScoreDatas = {}
    @@Score = 0
    def self.load
    end
    
    def self.save
    end
    
    def self.update()
    end
    
    def self.draw()
        GameWindow.draw_font( 0, Font.size, String + @@Score.to_s, Font)
    end
    
    def self.show_result()
    end
end