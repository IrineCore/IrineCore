USE [dngsm]
GO
/****** Object:  Table [dbo].[ExtGuildWarMail]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ExtGuildWarMail](
	[id] [int] NULL,
	[_ResultType] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_ClassID] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailID1] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_MailID2] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GuildFestPoint] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[_GuildPoint] [nvarchar](1024) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[PartitionKey] [nvarchar](255) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_UN_ExtGuildWarMail_id]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_ExtGuildWarMail_id] ON [dbo].[ExtGuildWarMail]
(
	[id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = ON, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
