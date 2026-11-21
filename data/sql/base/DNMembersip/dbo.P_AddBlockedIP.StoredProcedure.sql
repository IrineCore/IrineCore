USE [dnmembership]
GO
/****** Object:  StoredProcedure [dbo].[P_AddBlockedIP]    Script Date: 11/14/2026 8:51:10 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[P_AddBlockedIP]') AND type in (N'P', N'PC'))
BEGIN
EXEC dbo.sp_executesql @statement = N'CREATE PROCEDURE [dbo].[P_AddBlockedIP] AS' 
END
GO
/**
version : 7
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-01-04
Description : ??? IP ??? ?????.
return value :
0 = There is no error.
1 = The transaction is in an uncommittable state. Rolling back transaction.
100 = System error occurred.
101104 = ?? ?????? ??? ??? IP ?????.
history :
3 = StartDate, EndDate ?? ??? ?? ?? at 2010-07-07
6 = StartIP? EndIP?? ? ??? ?? ?? ?? ?? at 2010-11-25
7 = ?/??? ?? ?? at 2010-11-26
**/

ALTER PROCEDURE [dbo].[P_AddBlockedIP]
	  @vchStartIP varchar(15) --// ?? IP ??? ?? IP ??
	, @vchEndIP varchar(15) --// ?? IP ??? ? IP ??
	, @inyBlockedIPCode tinyint --// 
	, @nvcDescription nvarchar(300) = NULL --// ??? IP ??? ?? ??
	, @intAdminID int --// ?? IP ??? ??? ??? ID
	, @sdtStartDate smalldatetime = NULL --// ?? ?? ??, NULL ??? ?? ??
	, @sdtEndDate smalldatetime = NULL --// ?? ? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE
	@intReturnValue int,
	@intAffectedRowCount int,
	@intStartIP int,
	@intEndIP int,
	@intStartIPTemp int,
	@intEndIPTemp int,
	@dtmNow datetime;

DECLARE @tblDuplicatedIPBands table (seq int IDENTITY(1, 1) NOT NULL, EndIP int NOT NULL, StartIP int NOT NULL);

DECLARE @tblBlockedIPBands table (StartIP int NULL, EndIP int NULL);

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 4010001;
	GOTO ErrorHandler;
END;

BEGIN TRY
	DELETE dbo.BlockedIPs WHERE EndDate <= GETDATE();

	SET @dtmNow = GETDATE();
	SET @intStartIP = dbo.FN_IP2IPNumber(@vchStartIP);
	SET @intEndIP = dbo.FN_IP2IPNumber(@vchEndIP);

	SET @intAffectedRowCount = 1;

	WHILE @intAffectedRowCount > 0
	BEGIN
		SELECT @intStartIPTemp = (EndIP + 1)
		FROM (
			SELECT StartIP, EndIP
			FROM dbo.BlockedIPs WITH (NOLOCK)
			UNION ALL
			SELECT StartIP, EndIP
			FROM dbo.BlockedIPExceptions WITH (NOLOCK)
		) S
		WHERE (SELECT @intStartIP) BETWEEN StartIP AND EndIP;

		SET @intAffectedRowCount = @@ROWCOUNT;
		SET @intStartIP = COALESCE(@intStartIPTemp, @intStartIP);
	END;

	SET @intAffectedRowCount = 1;

	WHILE @intAffectedRowCount > 0
	BEGIN
		SELECT @intEndIPTemp = (StartIP - 1)
		FROM (
			SELECT StartIP, EndIP
			FROM dbo.BlockedIPs WITH (NOLOCK)
			UNION ALL
			SELECT StartIP, EndIP
			FROM dbo.BlockedIPExceptions WITH (NOLOCK)
		) S
		WHERE (SELECT @intEndIP) BETWEEN StartIP AND EndIP;

		SET @intAffectedRowCount = @@ROWCOUNT;
		SET @intEndIP = COALESCE(@intEndIPTemp, @intEndIP);
	END;

	/**_# ????? IP ?? ??? ?? ?????? ??? ??? IP ????, 101104? ?????.*/
	IF @intStartIPTemp > @intEndIPTemp
	BEGIN
		SET @intReturnValue = 101104;
		GOTO ErrorHandler;
	END;

	INSERT @tblDuplicatedIPBands (EndIP, StartIP)
	SELECT StartIP - 1, EndIP + 1
	FROM (
		SELECT StartIP, EndIP
		FROM dbo.BlockedIPs WITH (NOLOCK)
		UNION ALL
		SELECT StartIP, EndIP
		FROM dbo.BlockedIPExceptions WITH (NOLOCK)
	) S
	WHERE StartIP BETWEEN (SELECT @intStartIP) AND (SELECT @intEndIP);

	/**_# ????? IP ??? ?? ??? IP ?? ?? ??? ??? IP ??? ?????...*/
	IF @@ROWCOUNT > 0
	BEGIN
		INSERT @tblBlockedIPBands (StartIP, EndIP)
		SELECT B.StartIP, A.EndIP
		FROM @tblDuplicatedIPBands A
			FULL OUTER JOIN @tblDuplicatedIPBands B ON A.seq = (B.seq + 1);

		/**_## ?? ??? IP ?? ?? ??? ??? IP ??? ??? ??? IP ??? [BlockedIPs] ???? INSERT???.*/
		INSERT dbo.BlockedIPs (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate, StartDate, EndDate)
		SELECT COALESCE(StartIP, @intStartIP), COALESCE(EndIP, @intEndIP), @inyBlockedIPCode, @nvcDescription, @intAdminID, @dtmNow, @sdtStartDate, @sdtEndDate
		FROM @tblBlockedIPBands
		WHERE COALESCE(StartIP, @intStartIP) <= COALESCE(EndIP, @intEndIP);
	END;

	/**_# ????? IP ??? ?? ??? IP ?? ?? ??? ??? IP ??? ??? ???...*/
	ELSE
		/**_## [BlockedIPs] ???? INSERT???.*/
		INSERT dbo.BlockedIPs (StartIP, EndIP, BlockedIPCode, [Description], AdminID, RegisterDate, StartDate, EndDate)
		VALUES (@intStartIP, @intEndIP, @inyBlockedIPCode, @nvcDescription, @intAdminID, @dtmNow, @sdtStartDate, @sdtEndDate);

END TRY
BEGIN CATCH
	GOTO ErrorHandler;
END CATCH

RETURN 0;

ErrorHandler:
IF XACT_STATE() <> 0
	ROLLBACK TRANSACTION;
IF @intReturnValue IS NULL OR @intReturnValue = 0
	EXEC @intReturnValue = dbo.P_AddErrorLog;

RETURN @intReturnValue;
GO
