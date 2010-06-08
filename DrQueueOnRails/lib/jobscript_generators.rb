# This is an add-on to the Drqueue module
module Drqueue
  class Job
  
    # wrapper for specific script generators
    def generate_jobscript(renderer, *args)
      # blender animation
      if (renderer == "blender") && (args.size == 2)
      	scene, scriptdir = args
      	# user and group
      	file_owner = ENV['DQOR_USER'] + ":" + ENV['DQOR_GROUP']
      	kind = 1
      	output_path = self.blendersg(scene, scriptdir, kind)
      # blender single image
      elsif (renderer == "blender_image") && (args.size == 2)
      	scene, scriptdir = args
      	# user and group
      	file_owner = ENV['DQOR_USER'] + ":" + ENV['DQOR_GROUP']
      	kind = 2
      	output_path = self.blendersg(scene, scriptdir, kind)
      # mentalray animation
      elsif (renderer == "mentalray") && (args.size == 2)
      	scene, scriptdir = args
      	renderdir = scriptdir
      	image = camera = format = nil
      	# user and group
      	file_owner = ENV['DQOR_USER'] + ":" + ENV['DQOR_GROUP']
      	res_x = res_y = -1
      	kind = 1
        output_path = self.mentalraysg(scene, scriptdir, renderdir, image.to_s, file_owner.to_s, camera.to_s, res_x, res_y, format.to_s, kind)
      # mentalray single image
      elsif (renderer == "mentalray_image") && (args.size == 2)
      	scene, scriptdir = args
      	renderdir = scriptdir
      	image = camera = format = nil
      	# user and group
		file_owner = ENV['DQOR_USER'] + ":" + ENV['DQOR_GROUP']
      	res_x = res_y = -1
      	kind = 2
        output_path = self.mentalraysg(scene, scriptdir, renderdir, image.to_s, file_owner.to_s, camera.to_s, res_x, res_y, format.to_s, kind)
      # cinema4d animation
      elsif (renderer == "cinema4d") && (args.size == 2)
      	scene, scriptdir = args
      	# user and group
      	file_owner = ENV['DQOR_USER'] + ":" + ENV['DQOR_GROUP']
      	kind = 1
      	output_path = self.cinema_4dsg(scene, scriptdir, file_owner, kind)
      # luxrender animation
      elsif (renderer == "luxrender") && (args.size == 2)
      	scene, scriptdir = args
      	output_path = self.luxrendersg(scene, scriptdir)
      # luxrender through blender animation
      elsif (renderer == "blenderlux") && (args.size == 2)
      	scene, scriptdir = args
      	output_path = self.blenderluxsg(scene, scriptdir)
      # maya animation
      elsif (renderer == "maya") && (args.size == 3)
      	scene, scriptdir, mentalray = args
      	renderdir = scriptdir
      	projectdir = scriptdir
      	image = camera = format = nil
      	# user (group is found out by bin/chown_block)
      	file_owner = ENV['DQOR_USER']
      	res_x = res_y = -1
        output_path = self.mayasg(scene, projectdir, scriptdir, renderdir, image.to_s, file_owner.to_s, camera.to_s, res_x, res_y, format.to_s, mentalray)
        # vray animation
      elsif (renderer == "vray") && (args.size == 2)
      	scene, scriptdir = args
      	output_path = self.vraysg(scene, scriptdir)
      else
      	raise ArgumentError, "Wrong renderer and/or insufficient number of arguments."
      end
      
      return output_path
      
    end
    
  end
end
