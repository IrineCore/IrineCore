USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtCashTab]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtCashTab](
	[id] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MTNum] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_STNum] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Show] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_NameID] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[UIString_NameID] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_UN_ExtCashTab_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtCashTab_id] ON [dbo].[ExtCashTab]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
