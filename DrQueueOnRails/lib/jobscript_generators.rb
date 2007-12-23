# This is an add-on to the Drqueue module
module Drqueue
  class Job
  
    # wrapper for specific script generators
    def generate_jobscript(renderer, *args)
      # blender animation
      if (renderer == "blender") && (args.size == 2)
      	scene, scriptdir = args
      	kind = 1
      	output_path = self.blendersg(scene, scriptdir, kind)
      # blender single image
      elsif (renderer == "blender_image") && (args.size == 2)
      	scene, scriptdir = args
      	kind = 2
      	output_path = self.blendersg(scene, scriptdir, kind)
      # mentalray animation
      elsif (renderer == "mentalray") && (args.size == 2)
      	scene, scriptdir = args
      	renderdir = scriptdir
      	image = camera = format = nil
      	# userid and groupid
      	file_owner = `id -u drqueueonrails`.to_i.to_s + ":" + `id -g drqueueonrails`.to_i.to_s
      	res_x = res_y = -1
      	kind = 1
        output_path = self.mentalraysg(scene, scriptdir, renderdir, image.to_s, file_owner.to_s, camera.to_s, res_x, res_y, format.to_s, kind)
      # mentalray single image
      elsif (renderer == "mentalray_image") && (args.size == 2)
      	scene, scriptdir = args
      	renderdir = scriptdir
      	image = camera = format = nil
      	# userid and groupid
      	file_owner = `id -u drqueueonrails`.to_i.to_s + ":" + `id -g drqueueonrails`.to_i.to_s
      	res_x = res_y = -1
      	kind = 2
        output_path = self.mentalraysg(scene, scriptdir, renderdir, image.to_s, file_owner.to_s, camera.to_s, res_x, res_y, format.to_s, kind)
      # cinema4d animation
      elsif (renderer == "cinema4d") && (args.size == 2)
      	scene, scriptdir = args
      	# userid and groupid
      	file_owner = `id -u drqueueonrails`.to_i.to_s + ":" + `id -g drqueueonrails`.to_i.to_s
      	kind = 1
      	output_path = self.cinema_4dsg(scene, scriptdir, file_owner, kind)
      # cinema4d single image
      elsif (renderer == "cinema4d_image") && (args.size == 2)
      	scene, scriptdir = args
      	# userid and groupid
      	file_owner = `id -u drqueueonrails`.to_i.to_s + ":" + `id -g drqueueonrails`.to_i.to_s
      	kind = 2
      	output_path = self.cinema_4dsg(scene, scriptdir, file_owner, kind)
      else
      	raise ArgumentError, "Wrong renderer and/or unsufficient number of arguments."
      end
      
      return output_path
      
    end
    
  end
end
