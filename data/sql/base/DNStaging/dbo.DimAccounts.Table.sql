USE [dnstaging]
GO
/****** Object:  Table [dbo].[DimAccounts]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DimAccounts](
	[AccountID] [int] NOT NULL,
	[AccountName] [nvarchar](50) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[JoinIP] [int] NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
	[GenderCode] [tinyint] NULL,
	[BirthDate] [date] NULL,
	[SecondAuthCode] [tinyint] NULL,
	[NationalityCode] [tinyint] NULL,
	[PublisherCode] [tinyint] NULL,
	[ChannelPartnerCode] [tinyint] NULL,
 CONSTRAINT [PK_DimAccounts] PRIMARY KEY CLUSTERED 
(
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [IX_NN_DimAccounts_AccountName]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_DimAccounts_AccountName] ON [dbo].[DimAccounts]
(
	[AccountName] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
