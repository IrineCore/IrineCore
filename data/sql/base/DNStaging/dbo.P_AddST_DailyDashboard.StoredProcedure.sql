USE [dnstaging]
GO
/****** Object:  StoredProcedure [dbo].[P_AddST_DailyDashboard]    Script Date: 11/14/2026 9:01:06 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
version : 25
author : ???
e-mail : purumae@eyedentitygames.com
created date : 2010-05-06
description : ?? ?? ??? ??? ?????.
return value :
0 = ??? ????.
1 = ????? Commit? ? ?? ?????. ????? Rollback???.
100 = ??? ??? ???????. dbo.ErrorLogs ???? ?????.
history :
3 = DT ??? overflow ?? ?? ?? at 2010-07-25
5 = ?? ?? ?? at 2010-07-27
6 = ???? ?? - 20110302.wgkim
7 = ???? null ?? ??, ????? 0? ???? ???? ?? ???? ?? - 20110302.wgkim
8 = NationalityCode ? null ? ??? ?? ?? - 20110304.wgkim
8 = ???? ?? ?? ?? - 20110304.wgkim
11~14 = ????? VIEW ???? ???? ?? by ??? at 2012-02-07
15~16 = ??? ?? by ??? at 2012-10-18
18 = ??? ?? ???? ???? ?? by ??? at 2012-12-27
19 = DWC ???? ?? ????? ?? by ??? at 2013-02-19
20 = ?? ??? ???? ????? ?? by ??? at 2013-03-19
21 = ?? ??? ???? ???? ????? ?? by ??? at 2013-05-21
23 = PC? UV, ????? ?? by ??? at 2013-06-17
24 = CTE ???? ??? ?? by ??? at 2013-06-24
**/

CREATE PROCEDURE [dbo].[P_AddST_DailyDashboard]
	  @dtmBasisDate datetime  --// ?? ??
--WITH EXECUTE AS 'SPExecutor'
AS
SET NOCOUNT ON;
SET XACT_ABORT ON;

DECLARE @Temp_NationalityCode TABLE
( SeqNo             tinyint identity(1, 1)
, NationalityCode   tinyint NULL
);

DECLARE @intReturnValue int
      , @inyNationalityCode tinyint
      , @inySeqNo tinyint
      , @inyMaxSeqNo tinyint;
      
DECLARE @nvcStmt nvarchar(max) = N''
	, @nvcDBName nvarchar(50) = N'DNStaging'
	, @nvcTblName nvarchar(50) = N'AccountLogoutLogs'
	, @nvcTblName2 nvarchar(50) = N'CharacterLogoutLogs'
	, @nvcStartDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)
	, @nvcEndDate nvarchar(8) = CONVERT(nvarchar(8), @dtmBasisDate, 112)	
	, @nvcHint nvarchar(50) = N'NOLOCK'
	, @nvcStmtOUT nvarchar(max) = N''
	, @nvcStmtOUT2 nvarchar(max) = N''
	, @AvgDT smallint = NULL
	, @AvgDTForNewbie smallint = NULL
	, @intUVPCRoom int = NULL
	, @inbDTForPCRoom bigint = NULL;

SET @inySeqNo = 1;
SET @inyMaxSeqNo = 1;

/**_# Rollback and return if inside an uncommittable transaction.*/
IF XACT_STATE() = -1
BEGIN
	SET @intReturnValue = 1;
	GOTO ErrorHandler;
END

BEGIN TRY

    /**_# ????? ?????? ????. */
    INSERT @Temp_NationalityCode (NationalityCode)
    SELECT DISTINCT NationalityCode
      FROM dbo.DimAccounts WITH (NOLOCK);

    IF @@ROWCOUNT > 1
    BEGIN
        /***_# ????? ?? ?? ???? NULL ? ????? ???? ??? ?????. */
        IF EXISTS (SELECT * FROM @Temp_NationalityCode WHERE NationalityCode IS NULL)
        BEGIN
            RAISERROR(N'Error!! Invalid data : NationalityCode is NULL', 11, 1);
        END
    END

    /***_# ????? 0 ? ???? ???? ??? ?????. */
    IF EXISTS (SELECT * FROM @Temp_NationalityCode WHERE NationalityCode = 0)
    BEGIN
        RAISERROR(N'Error!! Invalid data : NationalityCode is 0', 11, 1);
    END

    SELECT @inyMaxSeqNo = MAX(SeqNo)
      FROM @Temp_NationalityCode;

	/** ????? ??? ?? by ??? at 2012-02-07 **/
	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT OUTPUT;

	EXEC dbo.P_MakeTableString
		  @nvcDBName = @nvcDBName
		, @nvcTblName = @nvcTblName2
		, @nvcStartDate = @nvcStartDate
		, @nvcEndDate = @nvcEndDate
		, @nvcHint = @nvcHint
		, @nvcStmtOUT = @nvcStmtOUT2 OUTPUT;

    WHILE (@inySeqNo <= @inyMaxSeqNo)
    BEGIN
    
        SELECT @inyNationalityCode = NationalityCode
          FROM @Temp_NationalityCode
         WHERE SeqNo = @inySeqNo;

		/** ?????? ??? ?? ? ???? ?? by ??? at 2012-02-07 **/
		SET @nvcStmt = N'
		  SELECT @AvgDT = SUM(CAST(DATEDIFF(SECOND, A.LoginDate, A.LogoutDate) AS bigint)) / COUNT(DISTINCT A.AccountID) / 60 
		    FROM (' + @nvcStmtOUT + N') A
			    INNER JOIN dbo.DimAccounts B ON (A.AccountID = B.AccountID AND (@inyNationalityCode IS NULL OR B.NationalityCode = @inyNationalityCode))
		    WHERE A.LoginDate < LogoutDate
			    AND A.LogoutDate >= DATEADD(DAY, DATEDIFF(DAY, 0, B.RegisterDate) + 1, 0)
			    AND A.LogoutDate >= @dtmBasisDate AND A.LogoutDate < (@dtmBasisDate + 1)
		    GROUP BY CAST(A.LogoutDate AS date)
		'
		EXEC sp_executesql @nvcStmt, N'
			  @inyNationalityCode tinyint
			, @dtmBasisDate datetime
			, @AvgDT smallint OUTPUT'
			, @inyNationalityCode = @inyNationalityCode
			, @dtmBasisDate = @dtmBasisDate
			, @AvgDT = @AvgDT OUTPUT;
			
		SET @nvcStmt = N'
		  SELECT @AvgDTForNewbie = SUM(CAST(DATEDIFF(SECOND, A.LoginDate, A.LogoutDate) AS bigint)) / COUNT(DISTINCT A.AccountID) / 60 
		    FROM (' + @nvcStmtOUT + N') A
			    INNER JOIN dbo.DimAccounts B ON (A.AccountID = B.AccountID AND (@inyNationalityCode IS NULL OR B.NationalityCode = @inyNationalityCode))
		    WHERE A.LoginDate < LogoutDate
			    AND A.LogoutDate >= DATEADD(DAY, DATEDIFF(DAY, 0, B.RegisterDate), 0) AND A.LogoutDate < DATEADD(DAY, DATEDIFF(DAY, 0, B.RegisterDate) + 1, 0)
			    AND A.LogoutDate >= @dtmBasisDate AND A.LogoutDate < (@dtmBasisDate + 1)
		    GROUP BY CAST(A.LogoutDate AS date)
		'
		EXEC sp_executesql @nvcStmt, N'
			  @inyNationalityCode tinyint
			, @dtmBasisDate datetime
			, @AvgDTForNewbie smallint OUTPUT'
			, @inyNationalityCode = @inyNationalityCode
			, @dtmBasisDate = @dtmBasisDate
			, @AvgDTForNewbie = @AvgDTForNewbie OUTPUT;

		SET @nvcStmt = N'
			SELECT @intUVPCRoom = COUNT(DISTINCT b.AccountID), @inbDTForPCRoom = SUM(DATEDIFF(MI, p.LoginDate, p.LogoutDate))
			FROM (' + @nvcStmtOUT2 + N') p
			INNER JOIN dbo.DimCharacters b WITH (NOLOCK) ON b.CharacterID = p.CharacterID
			WHERE p.LoginIP <> -1121974068 
              AND PCRoomFlag = 1
		'
		EXEC sp_executesql @nvcStmt, N'
			  @dtmBasisDate datetime
			, @intUVPCRoom int OUTPUT
			, @inbDTForPCRoom bigint OUTPUT'
			, @dtmBasisDate = @dtmBasisDate
			, @intUVPCRoom = @intUVPCRoom OUTPUT
			, @inbDTForPCRoom = @inbDTForPCRoom OUTPUT;

	    /**_# [ST_DailyDashboard] ???? INSERT???.*/
	     SELECT DUVA.AccountID
		  INTO #C1
          FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
               INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DUVA.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
         WHERE DUVA.BasisDate BETWEEN DATEADD(DAY, -6, @dtmBasisDate) AND DATEADD(DAY, -1, @dtmBasisDate);

	    SELECT DUVA.AccountID
		  INTO #C2
          FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
               INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DUVA.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
         WHERE DUVA.BasisDate = @dtmBasisDate;

	    SELECT DUVA.AccountID
		  INTO #C3
          FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
               INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DUVA.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
         WHERE DUVA.BasisDate <= @dtmBasisDate;

		SELECT AccountID
          INTO #C4
	      FROM #C3
	    GROUP BY AccountID
	    HAVING COUNT(*) > 1;

	    INSERT dbo.ST_DailyDashboard (BasisDate, RUCount, UVCount, RevisitType1, RevisitType2, AvgDT, AvgDTForNewbie, SaleAmount, NationalityCode, LeaveAccountCount, RevisitCount3, UVPCRoom, DTForPCRoom)
	    SELECT CAST(@dtmBasisDate AS date) AS BasisDate
		    , ISNULL((SELECT COUNT(*) FROM dbo.DimAccounts WITH (NOLOCK) WHERE RegisterDate >= @dtmBasisDate AND RegisterDate < (@dtmBasisDate + 1) AND (@inyNationalityCode IS NULL OR NationalityCode = @inyNationalityCode)), 0) AS RUCount
		    , ISNULL((SELECT COUNT(*)
                        FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
                             INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DUVA.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
                       WHERE DUVA.BasisDate = CAST(@dtmBasisDate AS date)), 0) AS UVCount
		    , ISNULL((SELECT COUNT(DISTINCT A.AccountID) FROM #C1 A INNER JOIN #C2 B ON A.AccountID = B.AccountID), 0) AS RevisitType1
		    , ISNULL((SELECT COUNT(*) FROM #C4), 0) AS RevisitType2
		    , ISNULL(@AvgDT, 0) AS AvgDT
		    , ISNULL(@AvgDTForNewbie, 0) AS AvgDTForNewbie
		    , ISNULL((
			    SELECT SUM(ISNULL(PO.PaidCashAmount, Price)) AS SaleAmount
			    FROM dbo.PurchaseOrders PO WITH (NOLOCK)
                     INNER JOIN dbo.DimCharacters DC WITH (NOLOCK) ON (PO.CharacterID = DC.CharacterID)
                     INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DC.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
			    WHERE PO.PayMethodCode = 2 AND PO.OrderStatusCode = 2 AND PO.OrderDate >= @dtmBasisDate AND PO.OrderDate < (@dtmBasisDate + 1) AND DC.AccountLevelCode = 0
		    ), 0) AS SaleAmount
            , @inyNationalityCode
		    , ISNULL((
				SELECT COUNT(*)
				FROM (SELECT DC.AccountID 
					FROM dbo.DimCharacters DC WITH (NOLOCK) 
						INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DC.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
					WHERE DC.AccountLevelCode = 0
					GROUP BY DC.AccountID
					HAVING MIN(DC.CreateDate) >= DATEADD(DD, -7, @dtmBasisDate) AND MIN(DC.CreateDate) < DATEADD(DD, -6, @dtmBasisDate)
					 AND MAX(DC.LastLoginDate) >= DATEADD(DD, -7, @dtmBasisDate) AND MAX(DC.LastLoginDate) < DATEADD(DD, -6, @dtmBasisDate)) X
			), 0) AS LeaveAccountCount
			, ISNULL((
				SELECT COUNT(*)
				FROM
				(
					SELECT DUVA.AccountID, COUNT(*) AS CNT
					FROM dbo.DailyUVAccounts DUVA WITH (NOLOCK)
						INNER JOIN dbo.DimAccounts DA WITH (NOLOCK) ON (DUVA.AccountID = DA.AccountID AND (@inyNationalityCode IS NULL OR DA.NationalityCode = @inyNationalityCode))
					WHERE DUVA.BasisDate >= DATEADD(DD, -1, CAST(@dtmBasisDate AS date))
					GROUP BY DUVA.AccountID HAVING COUNT(*)>1
				) X
			), 0) AS RevisitCount3
			, ISNULL(@intUVPCRoom, 0) AS UVPCRoom
			, ISNULL(@inbDTForPCRoom, 0) AS DTForPCRoom;

        SELECT @inySeqNo = @inySeqNo + 1;

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
