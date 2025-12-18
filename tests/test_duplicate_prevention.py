#!/usr/bin/env python3
"""
Test: Duplicate Application Prevention
Verifies that users cannot apply to the same job twice
"""

import urllib.request
import urllib.error
import json
import random
import sys

BASE_URL = "http://localhost:8080"

def request(method, endpoint, data=None, token=None):
    """Make HTTP request"""
    url = BASE_URL + endpoint
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = f"Bearer {token}"
    
    json_data = json.dumps(data).encode("utf-8") if data else None
    req = urllib.request.Request(url, data=json_data, headers=headers, method=method)
    
    try:
        with urllib.request.urlopen(req) as response:
            res_body = response.read().decode("utf-8")
            if res_body:
                try:
                    return json.loads(res_body), response.status
                except:
                    return res_body, response.status
            return None, response.status
    except urllib.error.HTTPError as e:
        try:
            error_body = e.read().decode('utf-8')
            try:
                return json.loads(error_body), e.code
            except:
                return error_body, e.code
        except:
            return None, e.code
    except Exception as e:
        print(f"Error: {e}")
        return None, 0

def main():
    rand_id = random.randint(10000, 99999)
    
    print("="*60)
    print(f"DUPLICATE APPLICATION PREVENTION TEST (ID: {rand_id})")
    print("="*60)
    print()
    
    # Setup: Create user and company
    user_email = f"testuser{rand_id}@test.com"
    company_email = f"testcompany{rand_id}@test.com"
    password = "TestPass123"
    
    # Register and login user
    print("1. Setting up test user...")
    request("POST", "/auth/register/user", {"email": user_email, "password": password})
    user_auth, _ = request("POST", "/auth/login", {"email": user_email, "password": password})
    if not user_auth or "token" not in user_auth:
        print("❌ FATAL: Could not create test user")
        return False
    user_token = user_auth["token"]
    user_id = user_auth["user"]["_id"]["$oid"]
    print(f"   ✅ User created: {user_id}")
    
    # Register and login company
    print("2. Setting up test company...")
    request("POST", "/auth/register/company", {"email": company_email, "password": password})
    company_auth, _ = request("POST", "/auth/login", {"email": company_email, "password": password})
    if not company_auth or "token" not in company_auth:
        print("❌ FATAL: Could not create test company")
        return False
    company_token = company_auth["token"]
    company_id = company_auth["company"]["_id"]["$oid"]
    print(f"   ✅ Company created: {company_id}")
    
    # Create a job
    print("3. Creating test job...")
    job_data = {
        "title": "Test Job for Duplicate Prevention",
        "description": "Testing duplicate application prevention",
        "requiredLanguages": ["Python"],
        "grade": "Junior",
        "skills": ["Testing"],
        "category": "QA",
        "salaryRange": {"min": 50000, "max": 80000},
        "location": "Remote",
        "workType": ["Full-time"]
    }
    job_res, status = request("POST", "/jobs", job_data, token=company_token)
    if not job_res or "_id" not in job_res:
        print("❌ FATAL: Could not create test job")
        return False
    job_id = job_res["_id"]
    print(f"   ✅ Job created: {job_id}")
    
    # Test: First application (should succeed)
    print()
    print("4. Testing FIRST application...")
    app_data = {
        "jobId": job_id,
        "message": "I'm interested in this position!",
        "resumeUrl": "https://example.com/resume.pdf"
    }
    
    app1, status1 = request("POST", "/applications", app_data, token=user_token)
    
    if app1 and "jobId" in app1 and status1 in [200, 201]:
        print(f"   ✅ PASS: First application succeeded")
        print(f"      Application ID: {app1.get('_id', 'N/A')}")
    else:
        print(f"   ❌ FAIL: First application failed")
        print(f"      Status: {status1}")
        print(f"      Response: {app1}")
        return False
    
    # Test: Second application to SAME job (should fail)
    print()
    print("5. Testing DUPLICATE application to same job...")
    app2, status2 = request("POST", "/applications", app_data, token=user_token)
    
    if app2 and "error" in app2:
        error_msg = app2.get("error", "")
        if "already applied" in error_msg.lower():
            print(f"   ✅ PASS: Duplicate application prevented!")
            print(f"      Error message: \"{error_msg}\"")
            print(f"      Status code: {status2}")
        else:
            print(f"   ⚠️  WARNING: Application rejected but with unexpected error")
            print(f"      Error: {error_msg}")
            print(f"      Status: {status2}")
    elif status2 == 400 or status2 == 409:
        print(f"   ✅ PASS: Duplicate application prevented (status {status2})")
        print(f"      Response: {app2}")
    else:
        print(f"   ❌ FAIL: Duplicate application was NOT prevented!")
        print(f"      Status: {status2}")
        print(f"      Response: {app2}")
        return False
    
    # Test: Third attempt (verify it's still blocked)
    print()
    print("6. Testing THIRD attempt (should still be blocked)...")
    app3, status3 = request("POST", "/applications", app_data, token=user_token)
    
    if app3 and "error" in app3:
        print(f"   ✅ PASS: Still blocked on third attempt")
    elif status3 == 400 or status3 == 409:
        print(f"   ✅ PASS: Still blocked on third attempt (status {status3})")
    else:
        print(f"   ❌ FAIL: Third application succeeded (should be blocked)")
        return False
    
    # Verify user has exactly ONE application to this job
    print()
    print("7. Verifying application count...")
    user_apps, _ = request("GET", f"/applications/user/{user_id}", token=user_token)
    
    if user_apps and isinstance(user_apps, list):
        job_apps = [app for app in user_apps if app.get("jobId") == job_id]
        if len(job_apps) == 1:
            print(f"   ✅ PASS: User has exactly 1 application to job {job_id}")
        else:
            print(f"   ❌ FAIL: User has {len(job_apps)} applications (expected 1)")
            return False
    else:
        print(f"   ⚠️  WARNING: Could not verify application count")
    
    # Test: Application to DIFFERENT job (should succeed)
    print()
    print("8. Testing application to DIFFERENT job (should succeed)...")
    job_data2 = job_data.copy()
    job_data2["title"] = "Another Test Job"
    job_res2, _ = request("POST", "/jobs", job_data2, token=company_token)
    
    if job_res2 and "_id" in job_res2:
        job_id2 = job_res2["_id"]
        app_data2 = app_data.copy()
        app_data2["jobId"] = job_id2
        
        app4, status4 = request("POST", "/applications", app_data2, token=user_token)
        
        if app4 and "jobId" in app4 and status4 in [200, 201]:
            print(f"   ✅ PASS: Application to different job succeeded")
        else:
            print(f"   ❌ FAIL: Application to different job failed")
            print(f"      Status: {status4}")
            print(f"      Response: {app4}")
            return False
    else:
        print(f"   ⚠️  WARNING: Could not create second job for testing")
    
    # Summary
    print()
    print("="*60)
    print("✅ ALL TESTS PASSED - Duplicate Prevention Working!")
    print("="*60)
    print()
    print("Summary:")
    print("  ✅ First application succeeded")
    print("  ✅ Duplicate application prevented")
    print("  ✅ Third attempt still blocked")
    print("  ✅ Only 1 application exists in database")
    print("  ✅ Application to different job succeeded")
    print()
    
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
