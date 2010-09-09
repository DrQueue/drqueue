require File.dirname(__FILE__) + '/../test_helper'
require 'profiles_controller'

# Re-raise errors caught by the controller.
class ProfilesController; def rescue_action(e) raise e end; end

class ProfilesControllerTest < Test::Unit::TestCase
  fixtures :profiles

  def setup
    @controller = ProfilesController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = profiles(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:profiles)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:profile)
    assert assigns(:profile).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:profile)
  end

  def test_create
    num_profiles = Profile.count

    post :create, :profile => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_profiles + 1, Profile.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:profile)
    assert assigns(:profile).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      Profile.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      Profile.find(@first_id)
    }
  end
end
