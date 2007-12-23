require File.dirname(__FILE__) + '/../test_helper'
require 'jobs_controller'

# Re-raise errors caught by the controller.
class JobsController; def rescue_action(e) raise e end; end

class JobsControllerTest < Test::Unit::TestCase
  fixtures :jobs

  def setup
    @controller = JobsController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = jobs(:first).id
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

    assert_not_nil assigns(:jobs)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:job)
    assert assigns(:job).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:job)
  end

  def test_create
    num_jobs = Job.count

    post :create, :job => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_jobs + 1, Job.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:job)
    assert assigns(:job).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      Job.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      Job.find(@first_id)
    }
  end
end
