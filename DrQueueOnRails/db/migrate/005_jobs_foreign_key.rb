class JobsForeignKey < ActiveRecord::Migration
  def self.up
  	add_column "jobs", "profile_id", :integer
  end

  def self.down
  	remove_column "jobs", "profile_id"
  end
end
