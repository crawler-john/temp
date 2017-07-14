            </div>
          </div>
    	</article>
      </div>
    </section>
    <footer class="footer">&copy; 2015 Altenergy Power System Inc.</footer>
    <script>
    <?php if(!strncmp($page, "home", 4) || !strncmp($page, "realtimedata", 12)): ?>
    /* 指定5分钟刷新一次 */
        function myrefresh() {
            window.location.reload();
        }
        setTimeout('myrefresh()',300000); 
    <?php endif; ?>     	
    /* 切换语言 */
        $(".chlang").click(function(){
            $.ajax({
                url : "<?php echo base_url('index.php/management/set_language');?>",
                type : "post",
                dataType : "json",
                data: "language=" + $(this).attr("id"),
            })
            setTimeout("location.reload();",500);//刷新页面
        });
    </script>
    </body>
</html>