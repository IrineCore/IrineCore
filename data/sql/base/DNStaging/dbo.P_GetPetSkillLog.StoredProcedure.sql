USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_GetPetSkillLog]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 3
author : ???
e-mail : ohtack@eyedentitygames.com
created date : 2011-10-21
description : ? ?? ??? ?????
result set : BasisDate, PetItemSerial, SkillID, WorldID
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = ????
**/

CREATE PROCEDURE [dbo].[P_GetPetSkillLog]
	  @inbPetItemSerial bigint --// ? ??? ???
	, @dt2StartDate		datetime2(0)	--// ?? ?? ??
	, @dt2EndDate		datetime2(0)	--// ?? ? ??

WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY
	/**_# [PetSkillLogs] ???? ?? ???.*/
	SELECT BasisDate, PetItemSerial, SkillID, WorldID
	  FROM dbo.PetSkillLogs WITH(NOLOCK)
	 WHERE BasisDate >= @dt2StartDate
       AND BasisDate <= @dt2EndDate
       AND PetItemSerial = @inbPetItemSerial
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
