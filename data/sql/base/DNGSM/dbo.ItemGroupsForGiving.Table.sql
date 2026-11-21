USE [dngsm]
GO
/****** Object:  Table [dbo].[ItemGroupsForGiving]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[ItemGroupsForGiving](
	[ItemGroupID] [int] IDENTITY(1,1) NOT NULL,
	[ItemGroupName] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[ItemGroupFKey] [varchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[AdminAuditID] [int] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
 CONSTRAINT [PK_ItemGroupsForGiving] PRIMARY KEY CLUSTERED 
(
	[ItemGroupID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
