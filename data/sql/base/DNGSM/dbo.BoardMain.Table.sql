USE [dngsm]
GO
/****** Object:  Table [dbo].[BoardMain]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BoardMain](
	[BoardSeq] [int] IDENTITY(1,1) NOT NULL,
	[UserIndex] [int] NOT NULL,
	[UserID] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[Subject] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[Contents] [nvarchar](max) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[WriteDate] [datetime] NOT NULL,
	[UpdateDate] [smalldatetime] NULL,
	[DeleteDate] [smalldatetime] NULL,
	[CommentCount] [int] NOT NULL,
	[AttachedFlag] [int] NOT NULL,
	[DeleteFlag] [tinyint] NOT NULL,
 CONSTRAINT [PK_BoardMain] PRIMARY KEY CLUSTERED 
(
	[BoardSeq] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING ON
GO
/****** Object:  Index [NIX_BoardMain_UserID]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE NONCLUSTERED INDEX [NIX_BoardMain_UserID] ON [dbo].[BoardMain]
(
	[UserID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
