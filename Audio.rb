module Audio
    @@music_list = []
    @@music_index = 0
    @@start_playing = false
    
    def self.load( music_folder)
        @@music_list = []
        @@music_index = 0
        @@start_playing = false
        Dir.glob(music_folder + "/*").each do |name|
            @@music_list.push( Ayame.new(name).predecode)
        end
    end
    
    def self.empty?()
        return @@music_list.empty?
    end
    
    def self.pause()
        @@music_list[@@music_index].pause(0)
    end
    
    def self.pause?()
        return @@music_list[@@music_index].pausing?
    end
    
    def self.restart()
        return @@music_list[@@music_index].resume(0)
    end
    
    def self.play( loop_num, feedout)
        @@music_list[@@music_index].play( loop_num, feedout)
        @@music_list[@@music_index].set_volume(70)
    end
    
    def self.playing?()
        return @@music_list[@@music_index].playing?
    end
    
    def self.update
        if !playing? && !pause?() then
            @@music_index += 1 if @@music_list.size > @@music_index
            @@music_index = 0 if @@music_list.size <= @@music_index
            play( 1, 0)
        end
        Ayame.update
    end
end