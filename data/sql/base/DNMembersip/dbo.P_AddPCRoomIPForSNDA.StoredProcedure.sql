USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddPCRoomIPForSNDA]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddPCRoomIPForSNDA]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddPCRoomIPForSNDA] AS' 
END
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-07-20
description : PC? IP? 1?? ?????. (?? ??? ?? ??)
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
2 = input param. @inyPCRoomGrade ?? at 2011-03-08
**/

ALTER PROCEDURE [dbo].[P_AddPCRoomIPForSNDA]
	  @vchIP varchar(15) --// IP ??
	, @inyPCRoomGrade tinyint = 1 --// PC? ??. 1=??, 2=?? ??? ????, 3=?? ??? ????, 4=?? ??? ????
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
	UPDATE dbo.PCRoomIPs
	SET PCRoomGrade = @inyPCRoomGrade
	WHERE IP = dbo.FN_IP2IPNumber(@vchIP);

	IF @@ROWCOUNT = 0
		INSERT dbo.PCRoomIPs (IP, PCRoomGrade) VALUES (dbo.FN_IP2IPNumber(@vchIP), @inyPCRoomGrade);
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
