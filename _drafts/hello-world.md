---
title: Hello, World!
layout: post
git_branch: hello_world
---

{% assign branch_url = page.git_branch | prepend: "/tree/" | prepend: site.github_url %}

Our goal for this first lesson is to create the "Hello, World!" equivalent of an NES ROM. To see it in action, load
[the finished product]({{ "/hello_world.nes" | prepend: branch_url }}) in your NES emulator of choice. 

