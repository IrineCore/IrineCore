USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtKeepBoxProvideItemTable]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtKeepBoxProvideItemTable](
	[id] [int] NULL,
	[_Type] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_CashSN] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Count] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_SealCount] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ItemOption] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_Dolis] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_TypeMID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtKeepBoxProvideItemTable_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtKeepBoxProvideItemTable_id] ON [dbo].[ExtKeepBoxProvideItemTable]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
