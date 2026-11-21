USE [dnworld]
GO
/****** Object:  StoredProcedure [dbo].[P_ModFarmActivationFlag]    Script Date: 11/14/2026 8:54:39 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 2
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2011-06-02
description : ??? ??/???? ???.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
103307 = ?? ?? ??? (??? ??) ???.
103308 = ?? ?? ????? ???? ????.
history :
2 = ??(?? ????) ?? ?? ????? ??? ????.     by 20110707.wgkim
**/

CREATE PROCEDURE [dbo].[P_ModFarmActivationFlag]
	  @bitActivationFlag bit --// 0=???, 1=??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @intReturnValue int, @dtmNowDate datetime = GETDATE(), @dtmInspectTime datetime, @intPeriodDate int;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

	IF @bitActivationFlag = 0
	BEGIN

		IF EXISTS (SELECT * FROM dbo.FarmInspect WITH (NOLOCK))
		BEGIN
			SET @intReturnValue = 103307;
			GOTO ErrorHandler;
		END

		INSERT dbo.FarmInspect (InspectTime) SELECT @dtmNowDate;

		-- ?? ?? ??
		UPDATE dbo.Farms
		SET ActivationFlag = @bitActivationFlag
		WHERE EnableFlag = 1;

	END
	ELSE IF @bitActivationFlag = 1
	BEGIN

		SELECT @dtmInspectTime = InspectTime FROM dbo.FarmInspect WITH (NOLOCK);

		IF @@ROWCOUNT = 0
		BEGIN
			SET @intReturnValue = 103308;
			GOTO ErrorHandler;
		END

		SELECT @intPeriodDate = DATEDIFF(MI, @dtmInspectTime, @dtmNowDate)

		-- ?????? ?????? ??? ????.
		BEGIN TRAN

			UPDATE dbo.FieldsForCharacter
			   SET PlantDate = DATEADD(MI, @intPeriodDate, PlantDate)
				 , LastUpdateDate = DATEADD(MI, @intPeriodDate, LastUpdateDate);

			TRUNCATE TABLE dbo.FarmInspect;

		COMMIT TRAN

		-- ?? ?? ??
		UPDATE dbo.Farms
		SET ActivationFlag = @bitActivationFlag
		WHERE EnableFlag = 1;

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
