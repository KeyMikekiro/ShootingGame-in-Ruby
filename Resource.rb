module Resource
    ResourceFile = "./Resource"
    @@images = {}
    @@sounds = {}
    @@musics = {}
    @@images_dir = {}
    @@sounds_dir = {}
    @@musics_dir = {}
    
    def self.init()
        search_file(ResourceFile)
    end
    
    def self.search_file( file_name)
        Dir.glob( file_name + "/*").each do | file|
            if file.include?(".png") then
                temp = file.scan(/\/(\w+)\./)
                raise "Overlap data: " + temp[0][0] + " Please change to Do NOT Overlap name." if @@images[temp[0][0]] != nil
                @@images.store( temp[0][0], Image.load(file))
                @@images_dir.store( temp[0][0], file)
            elsif file.include?(".ogg") then
                sort_audio_file( file)
            else
                search_file( file)
            end
        end
    end
    
    def self.sort_audio_file( file)
        if file.include?( "/music/") then
            temp = file.scan(/\/(\w+)\./)
            @@musics.store( temp[0][0], Ayame.new(file))
            @@musics_dir.store( temp[0][0], file)
        elsif file.include?( "/sound/") then
            temp = file.scan(/\/(\w+)\./)
            @@sounds.store( temp[0][0], Ayame.new(file))
            @@sounds_dir.store( temp[0][0], file)
        end
    end
    
    def self.image( name)
        return @@images[ name]
    end
    
    def self.sound( name)
        return @@sounds[ name]
    end
    
    def self.music( name)
        return @@musics[ name]
    end
    
    def self.image_dir( name)
        return @@images_dir[ name]
    end
    
    def self.sound_dir( name)
        return @@sounds_dir[ name]
    end
    
    def self.music_dir( name)
        return @@musics_dir[ name]
    end
end
