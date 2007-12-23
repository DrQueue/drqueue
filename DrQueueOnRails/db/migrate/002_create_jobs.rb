class CreateJobs < ActiveRecord::Migration
  def self.up
    create_table :jobs do |t|
    	t.column :queue_id, :integer
	 	t.column :renderer, :string
        t.column :sort, :string
    end
  end

  def self.down
    drop_table :jobs
  end
end
