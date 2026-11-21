USE [dnstaging]
GO
/****** Object:  UserDefinedFunction [dbo].[IF_NSplit]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
/**
author : doeyull.kim
e-mail : purumae@gmail.com
created date : 2009-04-02
description : ???? ?? ???? ???? Split?? ???? ?????.
**/

CREATE FUNCTION [dbo].[IF_NSplit] (
	@nvcString nvarchar(max),
	@nchDelimiter nchar(1)
)
RETURNS TABLE
AS
RETURN (
	SELECT ROW_NUMBER() OVER(ORDER BY n) AS seq, SUBSTRING(@nchDelimiter + @nvcString + @nchDelimiter, n + 1, CHARINDEX(@nchDelimiter, @nchDelimiter + @nvcString + @nchDelimiter, n + 1) - n - 1) AS string
	FROM dbo.Tally WITH (NOLOCK)
	WHERE n < (DATALENGTH(@nchDelimiter + @nvcString + @nchDelimiter) / 2) AND SUBSTRING(@nchDelimiter + @nvcString + @nchDelimiter, n, 1) = @nchDelimiter
)
GO
