USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPetSkillLog]    Script Date: 11/14/2026 8:54:37 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-10-18
description : ? ?? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history
1 = ????
2 = ????
3 = ????
**/

CREATE PROCEDURE [dbo].[P_AddPetSkillLog]
	  @inbPetItemSerial bigint --// ?? or ?? ??? ???
	, @intSkillID int --// ? ??ID 
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int
	   ,@intLogTableSuffix int
	   ,@dt2LogDate datetime2(0) = GETDATE();

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	SET @intLogTableSuffix = dbo.FN_LogTableSuffix(@dt2LogDate);
	/**_# [PetSkillLogs] ???? INSERT???.*/
	IF @intLogTableSuffix = 0
		INSERT dbo.PetSkillLogs0 (PetItemSerial, SkillID, BasisDate)
		VALUES (@inbPetItemSerial, @intSkillID, @dt2LogDate);
	ELSE IF @intLogTableSuffix = 1
		INSERT dbo.PetSkillLogs1 (PetItemSerial, SkillID, BasisDate)
		VALUES (@inbPetItemSerial, @intSkillID, @dt2LogDate);
	ELSE
		INSERT dbo.PetSkillLogs2 (PetItemSerial, SkillID, BasisDate)
		VALUES (@inbPetItemSerial, @intSkillID, @dt2LogDate);	
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
