USE [dnworld]
GO
/****** Object:  Table [dbo].[AlteiaSchedules]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AlteiaSchedules](
	[AlteiaStartDate] [datetime2](0) NULL,
	[AlteiaEndDate] [datetime2](0) NULL
) ON [DataFG1]
GO
INSERT [dbo].[AlteiaSchedules] ([AlteiaStartDate], [AlteiaEndDate]) VALUES (CAST(N'2014-11-08T08:00:00.0000000' AS DateTime2), CAST(N'2014-11-29T08:00:00.0000000' AS DateTime2))
GO
