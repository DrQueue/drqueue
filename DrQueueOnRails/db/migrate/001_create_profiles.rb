class CreateProfiles < ActiveRecord::Migration
  def self.up
    create_table :profiles do |t|
    	t.column :name, :string
	 	t.column :email, :string
        t.column :avatar, :string
        t.column :ldap_account, :string
        t.column :status, :string
    end
  end

  def self.down
    drop_table :profiles
  end
end
