```SQL
use finance1;
drop trigger if exists before_property_inserted;
-- 请在适当的地方补充代码，完成任务要求：
delimiter $$
CREATE TRIGGER before_property_inserted BEFORE INSERT ON property
FOR EACH ROW 
BEGIN
DECLARE msg VARCHAR(50);
IF new.pro_type = 1 AND NOT EXISTS (SELECT * FROM finances_product WHERE p_id = new.pro_pif_id) THEN
    SET msg = CONCAT("finances product #",new.pro_pif_id," not found!");
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = msg;
ELSEIF new.pro_type = 2 AND NOT EXISTS (SELECT * FROM insurance WHERE i_id = new.pro_pif_id) THEN
    SET msg = CONCAT("insurance #",new.pro_pif_id," not found!");
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = msg;
ELSEIF new.pro_type = 3 AND NOT EXISTS (SELECT * FROM fund WHERE f_id = new.pro_pif_id) THEN
    SET msg = CONCAT("fund #",new.pro_pif_id," not found!");
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = msg;
ELSEIF new.pro_type NOT IN (1,2,3) THEN
    SET msg = CONCAT("type ",new.pro_type," is illegal!");
    SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT = msg;
END IF;

END$$
 
delimiter ;

```