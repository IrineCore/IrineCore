USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModPetSkillExpand]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 4
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-10-18
description : ?? ?? ??? ?????
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103151 = ???? ???? ????.
history :
2 = ????
3 = ?????? ??? by ??? at 2011-12-08
4 = ? ?? ??? ??? ?? ?? by ??? at 2011-12-08
**/

CREATE PROCEDURE [dbo].[P_ModPetSkillExpand]
	  @inbPetItemSerial bigint --// ?? or ?? ??? ???
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
       ,@intNumberOfSkill int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	
	/**_# [PetEquipments]???? ????? UPDATE ???.*/
	UPDATE dbo.PetEquipments
	   SET ExpandSkillFlag = 1
	 WHERE PetItemSerial = @inbPetItemSerial AND PetEquipmentIndex = 255;

	/**_## ???? ??? 103151? ?????.*/
	IF @@ROWCOUNT = 0
	BEGIN
		SET @intReturnValue = 103151 ;
		GOTO ErrorHandler;
	END

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH;

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;

IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
