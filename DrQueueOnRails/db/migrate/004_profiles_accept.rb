class ProfilesAccept < ActiveRecord::Migration
  def self.up
  	add_column "profiles", "accepted", :integer, :default => 0
  end

  def self.down
  	remove_column "profiles", "accepted"
  end
end
